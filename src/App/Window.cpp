
#include <QLabel>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QVBoxLayout>
#include <QtMath>

#include <QCheckBox>
#include <QSlider>
#include <array>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Window.h"


Window::Window() noexcept
{

	auto layout = new QVBoxLayout();
	setLayout(layout);
	{
		const auto formatFPS = [](const auto value) {
			return QString("FPS: %1").arg(QString::number(value));
		};

		auto fps = new QLabel(formatFPS(0), this);
		fps->setStyleSheet("QLabel { color : white; }");
		connect(this, &Window::updateUI, [=] {
			fps->setText(formatFPS(ui_.fps));
		});
		layout->addWidget(fps, 1);
	}

	{
		auto morphLayout = new QHBoxLayout();

		{
			auto morphSlider = new QSlider();
			auto morphLabel = new QLabel("Morphing:");

			morphSlider->setRange(0, 100);
			morphSlider->setSingleStep(1);
			morphSlider->setOrientation(Qt::Horizontal);

			morphLayout->addWidget(morphLabel);
			morphLayout->addWidget(morphSlider, 2);

			connect(morphSlider, &QSlider::valueChanged, this, &Window::changeMorphParam);
		}

		{
			auto speedSlider = new QSlider();
			auto speedLabel = new QLabel("Speed");
			speedSlider->setRange(10, 200);
			speedSlider->setSingleStep(10);
			speedSlider->setOrientation(Qt::Horizontal);

			morphLayout->addWidget(speedSlider, 2);
			morphLayout->addWidget(speedLabel);

			connect(speedSlider, &QSlider::valueChanged, this, &Window::changeCameraSpeed);
		}

		//		{
		//			auto spotAngleSlider = new QSlider();
		//			auto spotAngleLabel = new QLabel("Angle:");
		//			spotAngleSlider->setRange(1, 60);
		//			spotAngleSlider->setValue(static_cast<int>(spotlightAngle_));
		//			spotAngleSlider->setSingleStep(1);
		//			spotAngleSlider->setOrientation(Qt::Horizontal);
		//
		//			morphLayout->addWidget(spotAngleSlider, 1);
		//			morphLayout->addWidget(spotAngleLabel);
		//
		//			connect(spotAngleSlider, &QSlider::valueChanged, this, &Window::changeSpotLightAngle);
		//		}
		layout->addLayout(morphLayout);
	}

	auto lightCheckBoxLayout = new QHBoxLayout();

	auto spotLightCheckBox = new QCheckBox();
	spotLightCheckBox->setChecked(false);
	spotLightCheckBox->setText("spot");

	lightCheckBoxLayout->addWidget(spotLightCheckBox, 1);
	connect(spotLightCheckBox, &QCheckBox::stateChanged, this, &Window::switchSpotLight);

	auto directionalLightCheckBox = new QCheckBox();
	directionalLightCheckBox->setChecked(false);
	directionalLightCheckBox->setText("directional");

	lightCheckBoxLayout->addWidget(directionalLightCheckBox, 2);
	connect(directionalLightCheckBox, &QCheckBox::stateChanged, this, &Window::switchDirectionalLight);


	auto spotAngleSlider = new QSlider();
	auto spotAngleLabel = new QLabel("Angle:");
	spotAngleSlider->setRange(1, 60);
	spotAngleSlider->setValue(static_cast<int>(spotlightAngle_));
	spotAngleSlider->setSingleStep(1);
	spotAngleSlider->setOrientation(Qt::Horizontal);

	lightCheckBoxLayout->addWidget(spotAngleSlider, 3);
	lightCheckBoxLayout->addWidget(spotAngleLabel);

	connect(spotLightCheckBox, &QCheckBox::stateChanged, spotAngleSlider, &QSlider::setEnabled);
	connect(spotAngleSlider, &QSlider::valueChanged, this, &Window::changeSpotLightAngle);


	//	lightCheckBoxLayout->addWidget(directionalLightCheckBox, 2);

	//	connect(spotLightCheckBox, &QCheckBox::stateChanged, this, &Window::switchSpotLight);

	//	connect(directionalLightCheckBox, &QCheckBox::stateChanged, this, &Window::switchDirectionalLight);
	layout->addLayout(lightCheckBoxLayout);


	setLayout(layout);

	timer_.start();
}

Window::~Window()
{
	{
		const auto guard = bindContext();
		texture_.reset();
		program_.reset();
	}
}

#include <filesystem>
void Window::onInit()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/diffuse.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/diffuse.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// ----------------------------------------------------------------
	gltfLoadModel("/home/ilsiyar/itmo-mse/compgraphic/computer_graphics_hw2/src/App/Models/cube.glb");
	vbos_ = gltfBindModel();
	// ---------------------------------------------

	texture_ = std::make_unique<QOpenGLTexture>(QImage("/home/ilsiyar/itmo-mse/compgraphic/computer_graphics_hw2/src/App/Textures/avatar.png"));
	texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	texture_->setWrapMode(QOpenGLTexture::WrapMode::Repeat);

	// Bind attributes
	program_->bind();


	uniforms_.enableDirectionalLight = program_->uniformLocation("enableDirectionalLight");
	uniforms_.enableSpotLight = program_->uniformLocation("enableSpotLight");
	uniforms_.morphingParam = program_->uniformLocation("morphingParam");
	uniforms_.model = program_->uniformLocation("modelMat");
	uniforms_.view = program_->uniformLocation("viewMat");
	uniforms_.projection = program_->uniformLocation("projMat");
	uniforms_.normal = program_->uniformLocation("normalMat");
	uniforms_.spotlightPosition = program_->uniformLocation("spotlightPosition");//spotlightSource
	uniforms_.spotlightDirection = program_->uniformLocation("spotlightDirection");
	uniforms_.spotlightAngle = program_->uniformLocation("spotLightAngle");

	// Release all
	program_->release();

	vao_.release();

	// Еnable depth test and face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform value

	// Activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	texture_->bind();

	// Draw
	draw();

	// Release VAO and shader program
	texture_->release();
	vao_.release();
	program_->release();

	++frameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize([[maybe_unused]] const size_t width, [[maybe_unused]] const size_t height)
{}

void Window::changeMorphParam(int state)
{
	morphingParam_ = 100 - state;
	update();
}

void Window::changeCameraSpeed(int speed)
{
	camera_.change_camera_speed(static_cast<float>(speed) / static_cast<float>(1000));
	update();
}

void Window::switchDirectionalLight(bool enable)
{
	enableDirectionalLight_ = enable;
	update();
}

void Window::switchSpotLight(bool enable)
{
	enableSpotLight_ = enable;
}

void Window::changeSpotLightAngle(int angle)
{
	spotlightAngle_ = static_cast<float>(angle);

	emit updateUI();
}
void Window::keyPressEvent(QKeyEvent * got_event)
{
	auto key = (Qt::Key)got_event->key();
	static std::map<Qt::Key, glm::vec3> keymap = {
		{Qt::Key_W, {1, 0, 0}},
		{Qt::Key_S, {-1, 0, 0}},
		{Qt::Key_A, {0, 0, -1}},
		{Qt::Key_D, {0, 0, 1}},
		{Qt::Key_X, {0, 1, 0}},
		{Qt::Key_C, {0, -1, 0}},
	};
	auto delta = keymap[key];
	camera_.updatePosition(delta.x, delta.z, delta.y);

	update();
}

void Window::mousePressEvent(QMouseEvent * got_event)
{
	mouseTrackStart_ = got_event->pos();
	mouseTrack_ = true;
}

void Window::mouseMoveEvent(QMouseEvent * got_event)
{
	if (mouseTrack_)
	{

		auto pos = got_event->pos();

		auto deltaX = mouseTrackStart_.x() - pos.x();
		auto deltaY = pos.y() - mouseTrackStart_.y();

		camera_.calculate_camera_front(static_cast<float>(deltaX * 0.1f), static_cast<float>(deltaY * 0.1f));

		mouseTrackStart_ = pos;

		update();
	}
}

void Window::mouseReleaseEvent([[maybe_unused]] QMouseEvent * got_event)
{
	mouseTrack_ = false;
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{std::move(callback)}
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}};
}

bool Window::gltfLoadModel(const char * filename)
{
	bool res = gltfLoader_.LoadBinaryFromFile(&this->gltfModel_, &err_, &warn_, filename);

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;

	if (!warn_.empty())
	{
		std::cout << "warning while loading model: " << warn_ << std::endl;
	}

	if (!err_.empty())
	{
		std::cout << "error while loading model:" << err_ << std::endl;
	}
	return res;
}

void Window::gltfBindModelNodes(std::map<int, GLuint> & vbos,
								tinygltf::Node & node)
{
	if ((node.mesh >= 0) && (static_cast<size_t>(node.mesh) < gltfModel_.meshes.size()))
	{
		gltfBindMesh(vbos, gltfModel_.meshes[node.mesh]);
	}

	for (int i: node.children)
	{
		gltfBindModelNodes(vbos, gltfModel_.nodes[i]);
	}
}

std::map<int, GLuint> Window::gltfBindModel()
{
	std::map<int, GLuint> vbos;

	const tinygltf::Scene & scene = gltfModel_.scenes[gltfModel_.defaultScene];
	for (int node: scene.nodes)
	{
		gltfBindModelNodes(vbos, gltfModel_.nodes[node]);
	}

	return vbos;
}

void Window::gltfBindMesh(std::map<int, GLuint> & vbos, tinygltf::Mesh & mesh)
{
	for (size_t i = 0; i < gltfModel_.bufferViews.size(); ++i)
	{
		const tinygltf::BufferView & bufferView = gltfModel_.bufferViews[i];
		if (bufferView.target == 0)
		{
			continue;
		}

		const tinygltf::Buffer & buffer = gltfModel_.buffers[bufferView.buffer];

		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		glBindBuffer(bufferView.target, vbo);

		glBufferData(bufferView.target, bufferView.byteLength,
					 &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	for (const auto & primitive: mesh.primitives)
	{
		for (auto & attrib: primitive.attributes)
		{
			tinygltf::Accessor accessor = gltfModel_.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(gltfModel_.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR)
			{
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first == "POSITION")
				vaa = 0;
			if (attrib.first == "NORMAL")
				vaa = 1;
			if (attrib.first == "TEXCOORD_0")
				vaa = 2;

			if (vaa > -1)
			{
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
									  accessor.normalized ? GL_TRUE : GL_FALSE,
									  byteStride, BUFFER_OFFSET(accessor.byteOffset));
			}
			else
				std::cout << "vaa missing: " << attrib.first << std::endl;
		}
	}
}

void Window::gltfDrawMesh(tinygltf::Mesh & mesh)
{
	for (auto primitive: mesh.primitives)
	{
		tinygltf::Accessor indexAccessor = gltfModel_.accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos_.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					   indexAccessor.componentType,
					   BUFFER_OFFSET(indexAccessor.byteOffset));
	}
}


void Window::gltfDrawModelNodes(tinygltf::Node & node)
{
	if ((node.mesh >= 0) && (static_cast<size_t>(node.mesh) < gltfModel_.meshes.size()))
	{
		gltfDrawMesh(gltfModel_.meshes[node.mesh]);
	}

	for (int i: node.children)
	{
		gltfDrawModelNodes(gltfModel_.nodes[i]);
	}
}

void Window::draw()
{
	model_ = glm::mat4(2.0f);
	view_ = glm::mat4(2.0f);
	projection_ = glm::mat4(2.0f);

	glClearColor(0.3, 0.3, 0.3, 1.0);// background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view_ = camera_.getView();
	GLint w, h;
	h = this->size().height();
	w = this->size().width();
	projection_ = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.01f, 100.0f);

	const auto normal_mv = glm::transpose(glm::inverse(view_ * model_));
	auto spot_direction = glm::vec3(0, 1, 2) - spotPosition_;

	{
		program_->setUniformValue(uniforms_.model, QMatrix4x4(glm::value_ptr(model_)).transposed());
		program_->setUniformValue(uniforms_.view, QMatrix4x4(glm::value_ptr(view_)).transposed());
		program_->setUniformValue(uniforms_.projection, QMatrix4x4(glm::value_ptr(projection_)).transposed());
		program_->setUniformValue(uniforms_.normal, QMatrix4x4(glm::value_ptr(normal_mv)).transposed());
		program_->setUniformValue(uniforms_.enableDirectionalLight, enableDirectionalLight_);
		program_->setUniformValue(uniforms_.enableSpotLight, enableSpotLight_);
		program_->setUniformValue(uniforms_.morphingParam, morphingParam_);
		program_->setUniformValue(uniforms_.spotlightPosition, QVector3D(spotPosition_.x, spotPosition_.y, spotPosition_.z));
		program_->setUniformValue(uniforms_.spotlightDirection, QVector3D(spot_direction.x, spot_direction.y, spot_direction.z));
		program_->setUniformValue(uniforms_.spotlightAngle, spotlightAngle_);
	}
	{
		const tinygltf::Scene & scene = gltfModel_.scenes[gltfModel_.defaultScene];
		for (int node: scene.nodes)
		{
			gltfDrawModelNodes(gltfModel_.nodes[node]);
		}
	}
}