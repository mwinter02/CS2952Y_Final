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


def run_coacd_on_file(input_path: Path, threshold: float = 0.05):
    """Run CoACD on a single mesh file and return the list of parts."""
    mesh = trimesh.load(str(input_path), force="mesh")

    # Ensure we have a mesh (triangulate if needed)
    if not isinstance(mesh, trimesh.Trimesh):
        # e.g., a Scene with multiple geometries – merge into one
        mesh = mesh.dump().sum()

    cmesh = coacd_mesh_from_trimesh(mesh)

    parts = coacd.run_coacd(cmesh, threshold=threshold)
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
        help="CoACD approximation threshold (0.01–1.0, default 0.05)",
    )

    args = parser.parse_args()
    in_path = Path(args.input)
    out_path = Path(args.output)

    if not in_path.is_file():
        raise SystemExit(f"Input file does not exist: {in_path}")

    out_path.parent.mkdir(parents=True, exist_ok=True)

    parts = run_coacd_on_file(in_path, threshold=args.threshold)
    write_parts_as_obj(parts, out_path)

    print(f"Wrote CoACD decomposition OBJ for {in_path} -> {out_path}")


if __name__ == "__main__":  # pragma: no cover
    main()
