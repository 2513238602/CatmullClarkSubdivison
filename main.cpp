#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"
#include "ModelReader.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <filesystem>
#include "arcball.h"
#include <map>
//settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
float  aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
bool enableImGuiMouseInput = true; // 默认情况下允许ImGui处理鼠标输入
bool wireframeMode = false;
bool subdivision = false;
bool catmullClark = false;
bool presubdivided = false;
bool catmullClarkSubdivided = false;
bool pureQuad = false;
bool quadSubdivided = false;
int catmullClarkSubdivisionLevel;
int maxSubdivisionLevel = 3;
string selectedPath = "models/2ballout.tri";

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
Arcball arcball;
bool updatemodel = false;

// timing
float deltaTime = 0.002f;	// time between current frame and last frame
float lastFrame = 0.0f;

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void render(ModelReader reader);
void SelectParentDirectory();
glm::mat4 quaternionToMatrix(const glm::quat& q);



using namespace std;
int main() {

	map<int, int> tree;

	ModelReader reader(selectedPath);
	reader.Load();



	//init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "catmullclark subdivision", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//initialise glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//set viewport
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//call the callbacks
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scroll_callback);

	//depth test
	glEnable(GL_DEPTH_TEST);

	// ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	//shaders
	Shader empty("shaders/empty.vs", "shaders/empty.fs");

	// timing
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;


	while (!glfwWindowShouldClose(window))
	{
		//clear 
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			// Start ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();


			// Set the width and height for the quarter of the screen
			int quarterWidth = SCR_WIDTH / 4;	
			int quarterHeight = SCR_HEIGHT;

			// Set the position and size for the Camera Parameters window
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight*0.1f), ImGuiCond_Always);
			ImGui::Begin("Camera Parameters");
			// Display camera parameters here using ImGui functions
			ImGui::Text("Camera Position: %.2f, %.2f, %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
			ImGui::Text("Camera Front: %.2f, %.2f, %.2f", camera.Front.x, camera.Front.y, camera.Front.z);
			ImGui::End();

			// Set the position and size for the Render Options window
			ImGui::SetNextWindowPos(ImVec2(0, quarterHeight * 0.1f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight*0.1f), ImGuiCond_Always);
			ImGui::Begin("Render Options");
			ImGui::Checkbox("Wireframe Mode", &wireframeMode);
			ImGui::End();

			// Set the position and size for the Camera Options window
			ImGui::SetNextWindowPos(ImVec2(0, quarterHeight * 0.2f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight*0.1f), ImGuiCond_Always);
			ImGui::Begin("Camera Options");
			ImGui::Checkbox("Move", &camera.enableInput);
			ImGui::End();

			//select path
			ImGui::SetNextWindowPos(ImVec2(0, quarterHeight * 0.3f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight * 0.1f), ImGuiCond_Always);
			ImGui::Begin("Selectpath");
			ImGui::Text("Selected Relative Path: %s", selectedPath.c_str());
			if (ImGui::Button("Select Parent Directory")) {
				SelectParentDirectory();
				reader.path = selectedPath;
				reader.Clear();
				reader.Load();
				//reader.Print();
			}
			ImGui::End();

			// subdivision controll
			ImGui::SetNextWindowPos(ImVec2(0, quarterHeight * 0.4f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight * 0.12f), ImGuiCond_Always);
			ImGui::Begin("subdivision Options");
			ImGui::Checkbox("Presubdivision", &subdivision);
			ImGui::Checkbox("catmullClark", &catmullClark);
			ImGui::SliderInt("Catmull-Clark Subdivision Level", &catmullClarkSubdivisionLevel, 1, maxSubdivisionLevel);
			ImGui::End();

			//controller
			ImGui::SetNextWindowPos(ImVec2(0, quarterHeight * 0.52f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(quarterWidth, quarterHeight * 0.15f), ImGuiCond_Always);
			ImGui::Begin("Print operation");
			if (ImGui::Button("Print Vertexs data")) {
				reader.PrintVertex();
			}
			if (ImGui::Button("Print Vertex Indices")) {
				reader.PrintVertexIndices();
			}
			if (ImGui::Button("Print halfEdges")) {
				reader.PrintHalfedges();
			}
			ImGui::End();


		//input
		processInput(window);
		glfwPollEvents();

		//render model
		empty.use();

		//set mvp
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		empty.setMat4("projection", projection);
		glm::mat4 view = camera.GetViewMatrix();
		empty.setMat4("view", view);
		glm::mat4 rotationMatrix = quaternionToMatrix(arcball.rotationQuat);
		empty.setMat4("model", rotationMatrix);

		//draw
		if (updatemodel == true) {
			rotationMatrix = quaternionToMatrix(arcball.rotationQuat);
			reader.ApplyTransformation(rotationMatrix);
			updatemodel = false;
		}

		if (wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (subdivision == true && presubdivided == false && catmullClarkSubdivided == false) {
			reader.Clear();
			reader.Load();
			reader.SubdivideTriangles();
			//reader.catmullClarkSubdivision();
			presubdivided = true;
		}

		if (catmullClark == true && catmullClarkSubdivided == false) {
			for (int i = 0; i < catmullClarkSubdivisionLevel; i++) {
				reader.catmullClarkSubdivision();
			}

			catmullClarkSubdivided = true;
			presubdivided = false;
		}

		if (catmullClark == false) {
			catmullClarkSubdivided = false;
		}
		
		if(subdivision == false && presubdivided == true){
			reader.Clear();
			reader.Load();
			presubdivided = false;
		}

		if (pureQuad == true && quadSubdivided == false) {
			reader.Clear();
			// 添加正方体的顶点
			reader.AddVertex(-1.0f, -1.0f, -1.0f);
			reader.AddVertex(1.0f, -1.0f, -1.0f);
			reader.AddVertex(1.0f, 1.0f, -1.0f);
			reader.AddVertex(-1.0f, 1.0f, -1.0f);
			reader.AddVertex(-1.0f, -1.0f, 1.0f);
			reader.AddVertex(1.0f, -1.0f, 1.0f);
			reader.AddVertex(1.0f, 1.0f, 1.0f);
			reader.AddVertex(-1.0f, 1.0f, 1.0f);

			// 添加正方体的面（按逆时针方向排列）
			reader.AddFace(0, 1, 2, 3);  // 底部
			reader.AddFace(0, 4, 5, 1);  // 顶部
			reader.AddFace(0, 3, 7, 4);  // 前面
			reader.AddFace(6, 5, 4, 7);  // 后面
			reader.AddFace(6, 7, 3, 2);  // 右侧
			reader.AddFace(6,5, 1, 2);  // 左侧

			// 添加顶点索引
			for (int i = 0; i < reader.faces.size(); ++i) {
				reader.AddVertexIndex(reader.faces[i].a);
				reader.AddVertexIndex(reader.faces[i].b);
				reader.AddVertexIndex(reader.faces[i].c);
				reader.AddVertexIndex(reader.faces[i].d);
			}

			quadSubdivided = true;
		}

		render(reader);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//release src
	glfwTerminate();

	return 0;
}

void render(ModelReader reader) {

	vector<float> vertices;
	vector<int> indices;
	vector<float> vertexColors; // Added for vertex colors

	if (subdivision == false) {
		for (int i = 0; i < reader.vertexList.size(); i++) {

			vertices.push_back(reader.vertexList[i].a);

			vertices.push_back(reader.vertexList[i].b);

			vertices.push_back(reader.vertexList[i].c);

		}

		for (int i = 0; i < reader.vertexIndices.size(); i++) {

			indices.push_back(reader.vertexIndices[i]);

		}
	}
	if(subdivision == true || pureQuad == true) {
		for (int i = 0; i < reader.vertexList.size(); i++) {

			vertices.push_back(reader.vertexList[i].a);

			vertices.push_back(reader.vertexList[i].b);

			vertices.push_back(reader.vertexList[i].c);
		}
		for (int i = 0; i < reader.vertexIndices.size(); i += 4) {

			int indexA = reader.vertexIndices[i];
			int indexB = reader.vertexIndices[i + 1];
			int indexC = reader.vertexIndices[i + 2];
			int indexD = reader.vertexIndices[i + 3];

			indices.push_back(indexA);
			indices.push_back(indexB);
			indices.push_back(indexC);
			indices.push_back(indexA);
			indices.push_back(indexC);
			indices.push_back(indexD);
		}
	}

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indices.size(), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;

		glfwGetCursorPos(window, &xpos, &ypos);
		arcball.onMouseDown(glm::vec2(xpos, ypos));
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		arcball.onMouseUp();
		updatemodel = true;
	}
}



void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	
	arcball.onMouseMove(glm::vec2(xpos, ypos));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!camera.enableInput)
		return;

	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window)
{
	if (!camera.enableInput)
		return;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

}
void SelectParentDirectory() {

	cin >> selectedPath;
}

glm::mat4 quaternionToMatrix(const glm::quat& q)
{
	glm::mat4 result(1.0f);

	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;

	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;

	float zz = q.z * q.z;
	float zw = q.z * q.w;

	result[0][0] = 1.0f - 2.0f * (yy + zz);
	result[1][0] = 2.0f * (xy - zw);
	result[2][0] = 2.0f * (xz + yw);

	result[0][1] = 2.0f * (xy + zw);
	result[1][1] = 1.0f - 2.0f * (xx + zz);
	result[2][1] = 2.0f * (yz - xw);

	result[0][2] = 2.0f * (xz - yw);
	result[1][2] = 2.0f * (yz + xw);
	result[2][2] = 1.0f - 2.0f * (xx + yy);

	return result;
}