#include "BasicScene.h"
#include <barycentric_coordinates.h>
#include <vector>

#include "igl/read_triangle_mesh.h"
#include "igl/per_vertex_normals.h"
#include "Mesh.h"
#include "Model.h"
#include <memory>
#include <algorithm>

using namespace cg3d;

void BasicScene::Init(float fov, int width, int height, float near, float far)
{
	std::cout << "click the model then space to simplify";
	std::cout << "you can use the arrow keys to change to the previous model before simplification";
	//camera
	SetNamedObject(camera, std::make_shared<Camera>, fov, float(width) / height, near, far);
	NewNamedObject(root, Movable::Create, shared_from_this());

	//skybox
	NewNamedObject(daylight, std::make_shared<Material>, "shaders/cubemapShader");
	daylight->AddTexture(0, "textures/cubemaps/Daylight Box_", 3);
	NewNamedObject(background, Model::Create, Mesh::Cube(), daylight, root);
	background->Scale(120, Axis::All);
	background->SetPickable(false);
	background->SetStatic();

	
	auto program = std::make_shared<Program>("shaders/basicShader");

	NewNamedObject(material, std::make_shared<Material>, program); // empty material	
	//SetNamedObject(cube, Model::Create, Mesh::Cube(), material, shared_from_this());
	material->AddTexture(0, "textures/bricks.jpg", 2);
	//NewNamedObject(torusMesh, ObjLoader::MeshFromObjFiles, "data/torus.obj");

	Eigen::MatrixXd OV;
	Eigen::MatrixXi OF;
	igl::read_triangle_mesh("data/bunny.off", OV, OF);
	addSimpMesh("bunny", OV, OF);
	std::vector<shared_ptr<cg3d::Mesh>>::iterator it = meshLib.begin();
	SetNamedObject(bunny, Model::Create, *it, material, shared_from_this());

	F = OF;
	V = OV;
	igl::edge_flaps(F, E, EMAP, EF, EI);
	C.resize(E.rows(), V.cols());
	Eigen::VectorXd costs(E.rows());
	// Q.clear();
	Q = {};
	EQ = Eigen::VectorXi::Zero(E.rows());
	{
		Eigen::VectorXd costs(E.rows());
		igl::parallel_for(E.rows(), [&](const int e)
		{
			double cost = e;
			Eigen::RowVectorXd p(1, 3);
			igl::shortest_edge_and_midpoint(e, V, F, E, EMAP, EF, EI, cost, p);
			C.row(e) = p;
			costs(e) = cost;
		}, 10000);
		for (int e = 0; e < E.rows(); e++)
		{
			Q.emplace(costs(e), e, 0);
		}
	}

	num_collapsed = 0;

	camera->Translate(4, Axis::Z);
	camera->Translate(1, Axis::Y);
	bunny->Scale(12);


	
	bunny->showWireframe = true;
	//igl::read_triangle_mesh("data/camel_b.obj", OV, OF);
	//add new mesh
	//addSimpMesh("camel", OV, OF);


	
}



void BasicScene::Update(const Program& program, const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, const Eigen::Matrix4f& model)
{
	Scene::Update(program, proj, view, model);
	program.SetUniform4f("lightColor", 1.0f, 1.0f, 1.0f, 0.5f);
	program.SetUniform4f("Kai", 1.0f, 1.0f, 1.0f, 1.0f);

	
}

void BasicScene::KeyCallback(cg3d::Viewport* viewport, int x, int y, int key, int scancode, int action, int mods)
{
	auto system = camera->GetRotation().transpose();

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) // NOLINT(hicpp-multiway-paths-covered)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_UP:
		{
			//go up in the meshlib
			if (currentMesh < meshLib.size() - 1)
				currentMesh += 1;
			std::vector<shared_ptr<cg3d::Mesh>>::iterator it = meshLib.begin();
			std::advance(it, currentMesh);
			pickedModel->SetMeshList({ *it });
		}
			break;
		case GLFW_KEY_DOWN:
		{
			//go down in the meshlib
			if (currentMesh > 0)
				currentMesh -= 1;
			std::vector<shared_ptr<cg3d::Mesh>>::iterator it2 = meshLib.begin();
			std::advance(it2, currentMesh);
			pickedModel->SetMeshList({ *it2 });
		}
			break;
		case GLFW_KEY_LEFT:
			camera->RotateInSystem(system, 0.1f, Axis::Y);
			break;
		case GLFW_KEY_RIGHT:
			camera->RotateInSystem(system, -0.1f, Axis::Y);
			break;
		case GLFW_KEY_W:
			camera->TranslateInSystem(system, { 0, 0.05f, 0 });
			break;
		case GLFW_KEY_S:
			camera->TranslateInSystem(system, { 0, -0.05f, 0 });
			break;
		case GLFW_KEY_A:
			camera->TranslateInSystem(system, { -0.05f, 0, 0 });
			break;
		case GLFW_KEY_D:
			camera->TranslateInSystem(system, { 0.05f, 0, 0 });
			break;
		case GLFW_KEY_B:
			camera->TranslateInSystem(system, { 0, 0, 0.05f });
			break;
		case GLFW_KEY_F:
			camera->TranslateInSystem(system, { 0, 0, -0.05f });
			break;
		case GLFW_KEY_K:
			pickedModel->showWireframe ^= 1;
			break;
		case GLFW_KEY_SPACE:
			{
			std::vector<shared_ptr<cg3d::Mesh>>::iterator it = meshLib.end();
			it--;
			simplify((*it)->name, (*it)->GetVertices(), (*it)->GetFaces());

			//go up in the meshlib
			if (currentMesh < meshLib.size() - 1)
				currentMesh += 1;
			std::vector<shared_ptr<cg3d::Mesh>>::iterator it2 = meshLib.begin();
			std::advance(it2, currentMesh);
			pickedModel->SetMeshList({ *it2 });
			}
			break;
		}
	}
}

void BasicScene::addSimpMesh(std::string name, Eigen::MatrixXd vertices, Eigen::MatrixXi faces) {
	std::vector<MeshData> dataList;
	Eigen::MatrixXd vertexNormals;
	Eigen::MatrixXd textureCoords;

	igl::per_vertex_normals(vertices, faces, vertexNormals);
	textureCoords = Eigen::MatrixXd::Zero(vertices.rows(), 2);
	dataList.push_back({ vertices, faces, vertexNormals, textureCoords });
	meshNum += 1;
	auto toADD{ std::make_shared<Mesh>(std::move(name + " " + std::to_string(meshNum)), dataList) };
	meshLib.push_back(toADD);
}

void BasicScene::simplify(std::string name, Eigen::MatrixXd vertices, Eigen::MatrixXi faces) {

	// collapse 10% of edges
	if (!Q.empty())
	{
		bool something_collapsed = false;
		// collapse edge
		const int max_iter = std::ceil(0.01 * Q.size());
		for (int j = 0; j < max_iter; j++)
		{
			if (!igl::collapse_edge(igl::shortest_edge_and_midpoint, V, F, E, EMAP, EF, EI, Q, EQ, C))
			{
				break;
			}
			something_collapsed = true;
			num_collapsed++;
		}

		if (something_collapsed)
		{	
			addSimpMesh(name,V, F);
			std::cout << "added simplified mesh";
		}
	}
}