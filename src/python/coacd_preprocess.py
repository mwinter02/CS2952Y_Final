import argparse
import json
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


def mesh_to_dict(m) -> dict:
    """Convert a CoACD part to a JSON-serializable dict.

    Supports several shapes:
      - coacd.Mesh-like object with .vertices and .faces
      - tuple/list: (vertices, faces)
      - dict with 'vertices' and 'faces' or 'triangles'
    Output layout:
      vertices: [x0, y0, z0, x1, y1, z1, ...]
      indices:  [i0, i1, i2, i3, i4, i5, ...]
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

    v = np.asarray(verts, dtype=np.float64).reshape(-1)
    f = np.asarray(faces, dtype=np.int32).reshape(-1)
    return {
        "vertices": v.tolist(),
        "indices": f.tolist(),
    }


def run_coacd_on_file(input_path: Path,
                      threshold: float = 0.05) -> dict:
    """Run CoACD on a single mesh file and return a JSON-serializable result.

    The structure is:
      {
        "source": "<original file path>",
        "threshold": <float>,
        "parts": [
          {"vertices": [...], "indices": [...]},
          ...
        ]
      }
    """
    mesh = trimesh.load(str(input_path), force="mesh")

    # Ensure we have a mesh (triangulate if needed)
    if not isinstance(mesh, trimesh.Trimesh):
        # e.g., a Scene with multiple geometries – merge into one
        mesh = mesh.dump().sum()

    cmesh = coacd_mesh_from_trimesh(mesh)

    parts = coacd.run_coacd(cmesh, threshold=threshold)

    return {
        "source": str(input_path),
        "threshold": float(threshold),
        "parts": [mesh_to_dict(p) for p in parts],
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Run CoACD on an input mesh file and write convex parts to a JSON "
            "file that can be loaded from C++ or JavaScript."
        )
    )
    parser.add_argument("input", help="Input mesh file (OBJ/STL/PLY/...)" )
    parser.add_argument(
        "output",
        help=(
            "Output JSON file to write decomposed meshes to. "
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

    result = run_coacd_on_file(in_path, threshold=args.threshold)

    with out_path.open("w", encoding="utf-8") as f:
        json.dump(result, f)

    print(f"Wrote CoACD decomposition for {in_path} -> {out_path}")


if __name__ == "__main__":  # pragma: no cover
    main()
