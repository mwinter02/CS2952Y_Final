# Mesh Decomposer

## User Guide
### Background
3D meshes in computer graphics and computational geometry often require decomposition into simpler components for various applications such as collision detection, physics simulations, and mesh simplification. This project provides a tool to decompose static 3D meshes into convex parts using the CoACD (Convex Approximation via Convex Decomposition) algorithm and Skeletal 3D meshes using Bone mesh hierarchy decomposition.

## User Instructions
1. **Input Requirements**: The input mesh should be in a standard 3D format (OBJ and FBX are supported).
2. **Running the Decomposition**:
   - For static meshes, use the CoACD algorithm by selecting the quality parameters and running the decomposition process.
   - For skeletal meshes, use the select Bone hierarchy method and press decompose.
3. **Output**: The output will be a set of convex parts saved in the same directory as the input mesh, with filenames having "_collider" suffix.
4. **Visualization**: Use any 3D viewer to visualize the decomposed parts.

## Basic example
### Static Mesh Decomposition
1. Load a static mesh (e.g., "Samples/bunny.obj").
2. Select the quality parameters for CoACD algorithm.
3. Press the "Decompose" button.
4. View your decomposed parts, use the render options tab to toggle visibility and draw modes.
5. You will find the output file named "Colliders/bunny_collider.obj" in the same directory as the input mesh.

### Skeletal Mesh Decomposition
1. Load a skeletal mesh (e.g., "Samples/walking.fbx").
2. Select the Bone hierarchy method.
   - Important Bones: Algorithmically selects what is considered important bones based on mesh influence. May not be optimal for all meshes.
   - All Bones: Uses all bones in the skeleton for decomposition.
   - Custom Selection: Manually select bones to guide the decomposition.
3. Press the "Decompose" button.
4. View your decomposed parts, use the render options tab to toggle visibility and draw modes.
5. Use animation buttons to preview the rigged collider with the original skeleton.
6. You will find the output file named "Colliders/walking_collider.obj" in the same directory as the input mesh. It will be rigged to the same skeleton as the input mesh.

## Build Instructions
First, run ```pip install coacd trimesh numpy pathlib``` to install python dependencies, then build the CmakeLists.txt with your favorite IDE.