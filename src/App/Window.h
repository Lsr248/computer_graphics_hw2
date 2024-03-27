#pragma once

#include <Base/GLWidget.hpp>

#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include <functional>
#include <memory>
// ------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <tinygltf/tiny_gltf.h>

#include "Camera.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class Window final : public fgl::GLWidget
{
	Q_OBJECT
public:
	Window() noexcept;
	~Window() override;

public:
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;

public:
	void mousePressEvent(QMouseEvent * got_event) override;
	void mouseMoveEvent(QMouseEvent * got_event) override;
	void mouseReleaseEvent(QMouseEvent * got_event) override;
	void keyPressEvent(QKeyEvent * got_event) override;

private:
	class PerfomanceMetricsGuard final
	{
	public:
		explicit PerfomanceMetricsGuard(std::function<void()> callback);
		~PerfomanceMetricsGuard();

		PerfomanceMetricsGuard(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard(PerfomanceMetricsGuard &&) = delete;

		PerfomanceMetricsGuard & operator=(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard & operator=(PerfomanceMetricsGuard &&) = delete;

	private:
		std::function<void()> callback_;
	};

private:
	bool gltfLoadModel(const char * filename);

	void gltfDrawMesh(tinygltf::Mesh & mesh);

	void gltfBindMesh(std::map<int, GLuint> & vbos, tinygltf::Mesh & mesh);
	std::map<int, GLuint> gltfBindModel();

	void gltfDrawModelNodes(tinygltf::Node & node);
	void gltfBindModelNodes(std::map<int, GLuint> & vbos, tinygltf::Node & node);

	void draw();

private:
	void changeCameraSpeed(int);
	void changeMorphParam(int);
	void switchDirectionalLight(bool);
	void switchSpotLight(bool);
	void changeSpotLightAngle(int);

private:
	[[nodiscard]] PerfomanceMetricsGuard captureMetrics();
signals:
	void updateUI();

private:
	struct {
		GLint model = -1;
		GLint view = -1;
		GLint projection = -1;
		GLint normal = -1;
		GLint morphingParam = -1;
		GLint enableDirectionalLight = -1;
		GLint enableSpotLight = -1;
		GLint spotlightAngle = -1;
		GLint spotlightPosition = -1;
		GLint spotlightDirection = -1;
	} uniforms_;

private:
	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};

	QOpenGLVertexArrayObject vao_;

private:
	glm::mat4 model_;
	glm::mat4 view_;

	glm::mat4 projection_;

private:
	std::unique_ptr<QOpenGLTexture> texture_;
	std::unique_ptr<QOpenGLShaderProgram> program_;
	QElapsedTimer timer_;


	size_t frameCount_ = 0;
	bool mouseTrack_ = false;
	QPoint mouseTrackStart_;
	Camera camera_{};


	float spotlightAngle_ = 20;


	// light params
	bool enableDirectionalLight_ = false;
	bool enableSpotLight_ = false;
	glm::vec3 spotPosition_ = glm::vec3(0.0, 5.0, 3.0);
	int morphingParam_ = 100;

	// model managing
	tinygltf::Model gltfModel_;
	tinygltf::TinyGLTF gltfLoader_;
	std::string err_;
	std::string warn_;
	std::map<int, GLuint> vbos_;


	struct {
		size_t fps = 0;
	} ui_;
	bool animated_ = false;
};