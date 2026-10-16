# 3D Solar System Simulation - CSE422 Project

A comprehensive, real-time 3D simulation of our solar system developed entirely in C++ using the OpenGL fixed-function pipeline for CSE422 (Computer Graphics Lab) at Daffodil International University.

## 📌 Project Information

* **Course:** CSE422 — Computer Graphics Lab
* **Team Members:** Imran Munshi, Md. Zamirul Islam Reyad
* **File:** `galactic_02.cpp`

## 🌐 Simulation Features

* **Hierarchical Transformations:** Accurately renders nested orbits and axial tilts for 8 planets and 6 moons (including Galilean moons and Titan).
* **Procedural Textures:** Algorithmically generates planet surfaces and Earth's night-side city lights using sine-wave interpolation and Gaussian functions without any external image files.
* **Advanced Lighting & Blending:** Implements the Phong lighting model, dual-pass rendering for Earth's day/night cycle, and additive blending for Saturn's rings and the solar corona.
* **Particle Systems & Galactic Motion:** Simulates a 300-object asteroid belt, a Halley-type comet with an 80-particle tail, and a solar system that dynamically moves through a fixed background of 1500 stars.
* **Interactive 2D HUD & 3D Labels:** Features a top-down mini-map, toggleable planet info panels, and real-time 3D-projected name tags using `gluProject()`.

## 🚀 How to Run

1. Ensure you have a C++ compiler (like MinGW or GCC) and the FreeGLUT library installed.
2. Open your terminal and compile the file using the following command:
   `g++ galactic_02.cpp -o solar.exe -lfreeglut -lopengl32 -lglu32`
3. Run the compiled executable by typing `./solar.exe` in your terminal or command prompt.
