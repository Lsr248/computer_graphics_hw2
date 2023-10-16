#pragma once

#include <Base/GLWidget.hpp>

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include <memory>

class Window final : public fgl::GLWidget
{
public:
	~Window() override;

public:
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;

private:
	GLint mvpUniform_ = -1;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	QMatrix4x4 model_;
	QMatrix4x4 view_;
	QMatrix4x4 projection_;

	std::unique_ptr<QOpenGLTexture> texture_ = nullptr;
	std::unique_ptr<QOpenGLShaderProgram> program_ = nullptr;
};
