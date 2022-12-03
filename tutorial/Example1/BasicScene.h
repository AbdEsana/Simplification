#pragma once

#include "Scene.h"
#include <igl/circulation.h>
#include <igl/collapse_edge.h>
#include <igl/edge_flaps.h>
#include <igl/decimate.h>
#include <igl/shortest_edge_and_midpoint.h>
#include <igl/writeOBJ.h>
#include <igl/parallel_for.h>
#include <igl/MshLoader.h>
#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <Eigen/Core>
#include <iostream>
#include <set>
#include "ObjLoader.h"
#include <utility>
#include <vector>
using namespace std;

class BasicScene : public cg3d::Scene
{
public:
    explicit BasicScene(std::string name, cg3d::Display* display) : Scene(std::move(name), display) {};
    void Init(float fov, int width, int height, float near, float far);
    void Update(const cg3d::Program& program, const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, const Eigen::Matrix4f& model) override;
	void KeyCallback(cg3d::Viewport* viewport, int x, int y, int key, int scancode, int action, int mods) override;
private:
	void addSimpMesh(std::string name, Eigen::MatrixXd vertices, Eigen::MatrixXi faces);
	void simplify(std::string name, Eigen::MatrixXd vertices, Eigen::MatrixXi faces);
	std::shared_ptr<cg3d::Model> bunny;
	std::vector<shared_ptr<cg3d::Mesh>> meshLib;
	Eigen::MatrixXd V ;
	Eigen::MatrixXi F ;
	Eigen::VectorXi EMAP;
	Eigen::MatrixXi E, EF, EI;
	igl::min_heap< std::tuple<double, int, int> > Q;
	Eigen::VectorXi EQ;
	Eigen::MatrixXd C;
	int num_collapsed;
	int meshNum = 0;
	int currentMesh = 0;
};
