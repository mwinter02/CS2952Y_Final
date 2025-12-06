import argparse
from pathlib import Path

import numpy as np
import trimesh  # type: ignore
import coacd  # Python package: pip install coacd


def coacd_mesh_from_trimesh(mesh: "trimesh.Trimesh") -> "coacd.Mesh":
    """Convert a trimesh.Trimesh to a coacd.Mesh.

    CoACD expects vertices (N, 3) and faces/triangles (M, 3) as numpy arrays.
    """
    vertices = np.asarray(mesh.vertices, dtype=np.float64)
    faces = np.asarray(mesh.faces, dtype=np.int32)
    return coacd.Mesh(vertices, faces)


def coacd_part_to_arrays(m):
    """Normalize a CoACD part into (vertices, faces) numpy arrays.

    Supports:
      - coacd.Mesh-like object with .vertices and .faces
      - tuple/list: (vertices, faces)
      - dict with 'vertices' and 'faces' or 'triangles'
    """
    # Case 1: object with attributes
    if hasattr(m, "vertices") and hasattr(m, "faces"):
        verts = m.vertices
        faces = m.faces
    # Case 2: tuple/list (vertices, faces)
    elif isinstance(m, (tuple, list)) and len(m) == 2:
        verts, faces = m
    # Case 3: dict-like with 'vertices' and 'faces'/'triangles'
    elif isinstance(m, dict):
        verts = m.get("vertices")
        faces = m.get("faces", m.get("triangles"))
        if verts is None or faces is None:
            raise TypeError(f"Unsupported CoACD part dict structure: keys={list(m.keys())}")
    else:
        raise TypeError(f"Unsupported CoACD part type: {type(m)}")

    v = np.asarray(verts, dtype=np.float64)
    f = np.asarray(faces, dtype=np.int32)
    return v, f


def write_parts_as_obj(parts, out_path: Path) -> None:
    """Write CoACD parts to a single OBJ file with multiple objects.

    Each part is written as a separate object named Collider_XX, where XX
    is the zero-padded part index (e.g., Collider_00, Collider_01, ...).
    """
    lines = []
    vertex_offset = 0  # OBJ indices are 1-based and global in this file

    # Determine zero padding width from number of parts
    num_parts = len(parts)
    pad_width = max(2, len(str(max(num_parts - 1, 0))))

    for idx, part in enumerate(parts):
        vertices, faces = coacd_part_to_arrays(part)

        obj_name = f"Collider_{idx:0{pad_width}d}"

        lines.append(f"\n#\n# Object {obj_name}\n#\n")
        lines.append(f"o {obj_name}")

        # vertices: (N, 3)
        for v in vertices:
            lines.append(f"v {v[0]} {v[1]} {v[2]}")

        # faces: (M, 3) indices assumed 0-based from CoACD
        for f in faces:
            i, j, k = f
            lines.append(
                f"f {i + 1 + vertex_offset} {j + 1 + vertex_offset} {k + 1 + vertex_offset}"
            )

        vertex_offset += vertices.shape[0]

    out_path.write_text("\n".join(lines), encoding="utf-8")


def run_coacd_on_file(
    input_path: Path,
    threshold: float = 0.05,
    max_convex_hull: int = -1,
    preprocess_resolution: int = 50,
    resolution: int = 2000,
    mcts_nodes: int = 20,
    mcts_iterations: int = 150,
    mcts_max_depth: int = 3,
    pca: bool = False,
    approximate_mode: str = "ch",
    extrude_margin: float = 0.0,
):
    """Run CoACD on a single mesh file and return the list of parts.

    Args:
        threshold: Concavity threshold (0.01-1.0). Lower = more accurate but more parts.
        max_convex_hull: Maximum number of convex hulls (-1 = no limit).
        preprocess_resolution: Resolution for preprocessing. Higher = more detail.
        resolution: Sampling resolution for manifold. Higher = more detailed hulls.
        mcts_nodes: MCTS sampling nodes. Higher = better quality but slower.
        mcts_iterations: MCTS iterations. Higher = better quality but slower.
        mcts_max_depth: MCTS max depth for search.
        pca: Use PCA for initial partitioning.
        approximate_mode: Approximation mode - "ch" for convex hull, "box" for bounding boxes.
        extrude_margin: Percentage-based scale factor. 0.1 = 10% expansion, -0.1 = 10% contraction.
    """
    mesh = trimesh.load(str(input_path), force="mesh")

    # Ensure we have a mesh (triangulate if needed)
    if not isinstance(mesh, trimesh.Trimesh):
        # e.g., a Scene with multiple geometries – merge into one
        mesh = mesh.dump().sum()

    cmesh = coacd_mesh_from_trimesh(mesh)

    # NOTE: CoACD does NOT have extrude/extrude_margin parameters
    # We apply scaling post-processing instead
    parts = coacd.run_coacd(
        cmesh,
        threshold=threshold,
        max_convex_hull=max_convex_hull,
        preprocess_resolution=preprocess_resolution,
        resolution=resolution,
        mcts_nodes=mcts_nodes,
        mcts_iterations=mcts_iterations,
        mcts_max_depth=mcts_max_depth,
        pca=pca,
    )

    # Convert parts to arrays first (normalize the format)
    parts = [coacd_part_to_arrays(part) for part in parts]

    # Post-process parts based on approximate mode FIRST (before extrusion)
    if approximate_mode == "box":
        # Convert each convex hull to its axis-aligned bounding box
        box_parts = []
        for vertices, faces in parts:
            # Compute AABB (axis-aligned bounding box)
            min_bounds = vertices.min(axis=0)
            max_bounds = vertices.max(axis=0)

            # Create box vertices (8 corners)
            box_vertices = np.array([
                [min_bounds[0], min_bounds[1], min_bounds[2]],
                [max_bounds[0], min_bounds[1], min_bounds[2]],
                [max_bounds[0], max_bounds[1], min_bounds[2]],
                [min_bounds[0], max_bounds[1], min_bounds[2]],
                [min_bounds[0], min_bounds[1], max_bounds[2]],
                [max_bounds[0], min_bounds[1], max_bounds[2]],
                [max_bounds[0], max_bounds[1], max_bounds[2]],
                [min_bounds[0], max_bounds[1], max_bounds[2]],
            ], dtype=np.float64)

            # Create box faces (12 triangles, 2 per face)
            # All faces use counter-clockwise winding when viewed from outside
            box_faces = np.array([
                # Bottom face (z = min, normal pointing down -Z)
                [0, 2, 1], [0, 3, 2],
                # Top face (z = max, normal pointing up +Z)
                [4, 5, 6], [4, 6, 7],
                # Front face (y = min, normal pointing -Y)
                [0, 1, 5], [0, 5, 4],
                # Back face (y = max, normal pointing +Y)
                [2, 3, 7], [2, 7, 6],
                # Left face (x = min, normal pointing -X)
                [0, 4, 7], [0, 7, 3],
                # Right face (x = max, normal pointing +X)
                [1, 2, 6], [1, 6, 5],
            ], dtype=np.int32)

            box_parts.append((box_vertices, box_faces))

        parts = box_parts

    # Post-process: Apply extrusion by scaling hulls from their centroid
    # This is done AFTER box conversion so it applies to both convex hulls and boxes
    if extrude_margin != 0.0:
        processed_parts = []
        print(f"\nApplying extrusion margin: {extrude_margin}")
        for idx, (vertices, faces) in enumerate(parts):
            # Calculate centroid
            centroid = vertices.mean(axis=0)

            # Use percentage-based scaling instead of absolute margin
            # This ensures consistent behavior across different mesh scales
            # extrude_margin is now interpreted as a percentage:
            #   0.1 = 10% expansion, -0.1 = 10% contraction
            scale_factor = 1.0 + extrude_margin

            # Debug output
            if idx < 3:  # Only print first 3 parts to avoid spam
                print(f"  Part {idx}: scale_factor={scale_factor:.4f} ({extrude_margin*100:+.1f}%)")

            # Scale vertices from centroid
            scaled_vertices = centroid + (vertices - centroid) * scale_factor

            processed_parts.append((scaled_vertices, faces))

        parts = processed_parts

    return parts


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Run CoACD on an input mesh file and write convex parts to an OBJ "
            "file that can be loaded from C++ or JavaScript."
        )
    )
    parser.add_argument("input", help="Input mesh file (OBJ/STL/PLY/...)" )
    parser.add_argument(
        "output",
        help=(
            "Output OBJ file to write decomposed meshes to. "
            "Directory will be created if it does not exist."
        ),
    )
    parser.add_argument(
        "--threshold",
        type=float,
        default=0.05,
        help="CoACD approximation threshold (0.01–1.0, default 0.05). Lower = more accurate but more parts.",
    )
    parser.add_argument(
        "--max-convex-hull",
        type=int,
        default=-1,
        help="Maximum number of convex hulls to generate (-1 = no limit, default -1).",
    )
    parser.add_argument(
        "--preprocess-resolution",
        type=int,
        default=50,
        help="Resolution for preprocessing (default 50). Higher = more detail but slower.",
    )
    parser.add_argument(
        "--resolution",
        type=int,
        default=2000,
        help="Sampling resolution for manifold (default 2000). Higher = more detailed hulls.",
    )
    parser.add_argument(
        "--mcts-nodes",
        type=int,
        default=20,
        help="MCTS sampling nodes (default 20). Higher = better quality but slower.",
    )
    parser.add_argument(
        "--mcts-iterations",
        type=int,
        default=150,
        help="MCTS iterations (default 150). Higher = better quality but slower.",
    )
    parser.add_argument(
        "--mcts-max-depth",
        type=int,
        default=3,
        help="MCTS max depth for search (default 3).",
    )
    parser.add_argument(
        "--pca",
        action="store_true",
        help="Use PCA for initial partitioning (default False).",
    )
    parser.add_argument(
        "--approximate-mode",
        type=str,
        default="ch",
        choices=["ch", "box"],
        help="Approximation mode: 'ch' for convex hull (default), 'box' for axis-aligned bounding boxes.",
    )
    parser.add_argument(
        "--extrude-margin",
        type=float,
        default=0.0,
        help="Percentage-based scaling. 0.1 = 10%% expansion, -0.1 = 10%% contraction (default 0.0).",
    )

    args = parser.parse_args()
    in_path = Path(args.input)
    out_path = Path(args.output)

    if not in_path.is_file():
        raise SystemExit(f"Input file does not exist: {in_path}")

    out_path.parent.mkdir(parents=True, exist_ok=True)

    parts = run_coacd_on_file(
        in_path,
        threshold=args.threshold,
        max_convex_hull=args.max_convex_hull,
        preprocess_resolution=args.preprocess_resolution,
        resolution=args.resolution,
        mcts_nodes=args.mcts_nodes,
        mcts_iterations=args.mcts_iterations,
        mcts_max_depth=args.mcts_max_depth,
        pca=args.pca,
        approximate_mode=args.approximate_mode,
        extrude_margin=args.extrude_margin,
    )
    write_parts_as_obj(parts, out_path)

    print(f"Wrote CoACD decomposition OBJ for {in_path} -> {out_path}")
    print(f"  Mode: {'Convex Hulls' if args.approximate_mode == 'ch' else 'Bounding Boxes'}")
    print(f"  Parts: {len(parts)}")
    if args.extrude_margin != 0.0:
        print(f"  Extrusion: {args.extrude_margin*100:+.1f}%")


if __name__ == "__main__":  # pragma: no cover
    main()
