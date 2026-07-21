# Physics Sandbox

A physics sandbox written from scratch in C++ using Raylib to explore collision detection, orbital mechanics, fluid simulation, rendering techniques, and numerical methods.

## Features

### Rigid Body Collision Simulator
- Impulse-based collision resolution
- Dynamic and static rigid bodies
- Adjustable mass and restitution
- Drag-and-launch spawning system
- Velocity vector visualization
- Pause and frame stepping controls
- Interactive object selection and inspection

### Orbital Simulator
- Newtonian gravitational simulation
- Stable circular orbit generation
- N-body gravitational interactions
- Real-time orbit prediction
- Velocity vector visualization
- Adjustable time scaling
- Persistent orbital trail rendering
- Interactive body spawning and selection

### Fluid Simulator 
- Particle-based fluid simulation
- Density and pressure calculations
- Uniform spatial grid acceleration structure
- Viscosity modeling
- Buoyancy and rigid body interaction
- Rotating floating objects
- Metaball shader rendering
- Interactive mouse stirring
- Real-time fluid rendering at interactive frame rates

## Wind Simulator
- Adjustable wind speed with real-time particle flow visualization
- Multiple obstacle shapes including circle, square, diamond, and NACA airfoil
- Particle deflection around obstacles to illustrate basic aerodynamic flow
- Surface-following collision response using normals and tangential velocity projection
- Wake generation with adjustable turbulence and downstream disturbances
- Particle trails for visualizing streamlines and flow patterns
- Real-time obstacle interaction with continuous particle spawning and recycling
- Interactive controls for comparing flow behavior across different obstacle geometries
- Built using C++ and raylib with a custom particle-based physics system

## Built With

- C++
- Raylib
- CMake
- GLSL shaders

## Project Goals

This project is being developed as a long-term learning project to explore:

- Physics simulation
- Collision detection
- Numerical integration
- Rendering
- Interactive UI
- Software architecture

## Future Additions

- Navier-Stokes implementation
- Soft Body/Spring simulation
- Double pendulum simulation


## Screenshots
-Main Menu

![Main Menu](screenshots/menu-v2.png)


## Collision Sandbox

![Collision Sandbox](screenshots/collision-sim-demo.gif)


## Ortbital Sandbox

![Orbital Sandbox](screenshots/orbital-sim-demo.gif)


## Fluid Sandbox

![Fluid Sandbox](screenshots/fluid-sim-demo.gif)

## Wind Sandbox

![Fluid Sandbox](screenshots/wind-sim-demo.gif)


## How to Build

```bash
mkdir build
cd build
cmake ..
make
./PhysicsEngine
```

## Author

Nicholas Graves
