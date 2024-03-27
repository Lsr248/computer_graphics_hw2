#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Camera {
	glm::vec3 cameraPos_ = {7, 2, 0};
	glm::vec3 cameraFront_ = {0, 0, 0};
	glm::vec3 cameraUp_ = glm::vec3(0, 1, 0);

	float cameraSpeed_ = 0.01;
	float yaw_ = -180.;
	float pitch_ = -13.6;

	constexpr static float ROTATION_SPEED = 0.1f;

	Camera()
	{
		calculate_camera_front(yaw_, pitch_);
	}


	void calculate_camera_front(float deltaYaw, float deltaPitch)
	{
		yaw_ += deltaYaw * ROTATION_SPEED;
		pitch_ += deltaPitch * ROTATION_SPEED;

		yaw_ = std::fmod(yaw_, 360.0f);
		pitch_ = std::clamp(pitch_, -85.f, +85.f);

		float cosYaw = glm::cos(glm::radians(yaw_));
		float sinYaw = glm::sin(glm::radians(yaw_));
		float cosPitch = glm::cos(glm::radians(pitch_));
		float sinPitch = glm::sin(glm::radians(pitch_));

		cameraFront_ = glm::normalize(glm::vec3(
			cosYaw * cosPitch,
			sinPitch,
			sinYaw * cosPitch));
	}

	void updatePosition(float deltaForward, float deltaRightward, float deltaUpward)
	{
		auto right = glm::normalize(glm::cross(cameraFront_, cameraUp_));
		cameraPos_ += ((deltaForward * cameraFront_) + (deltaRightward * right) + (deltaUpward * cameraUp_)) * cameraSpeed_;
	}

	[[nodiscard]] glm::mat4 getView() const
	{
		return glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
	}

	void change_camera_speed(float new_sped)
	{
		cameraSpeed_ = new_sped;
	}
};