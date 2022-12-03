the whole simplification happens in Example1 because i ran into problems with the powershell, not much has changed from the engine, using the libigl tutorial 703 decimation, i added the fields into basicScene.h then added an extra field which is a vector of meshes which i will use to append a new mesh after simplification. *note you must click the model then by pressing space it will simplify by collapsing 10% of edges, as a new mesh which is added to the meshLib(vector of meshes). you can use the down key to go to previous step when the mesh wasnt  as simplified and the up arrow key to go deeper into the simplification if you want to add another simplify click space.
also added you can press "K" when a model is selected (crashes if no model is selected) it will toggle wireframe.


# EngineForAnimationCourse
Graphic Engine based on Libigl

For compiling:
1. Clone or download the project
2. Download Cmake from the link in the site
3. Run Cmake GUI, choose the project folder and destination folder for the cpp project files, press configure choose compiler (VS2019 for example). After configuration finished successfully press configure again and then press generate. 
4. If everything passes successfully click the "launch project" button or go to the build folder and launch the project from there. 

Notes for building and running with CLion:
1. Let CMake pick the generator (do not use Ninja generator!).
2. To run the demo edit Demo_bin configuration and put $CMakeCurrentGenerationDir$ in the working directory.
3. To share the build directory with VS choose build directory to be "build"
