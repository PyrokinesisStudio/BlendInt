/*
 * This file is part of BlendInt (a Blender-like Interface Library in
 * OpenGL).
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is free
 * software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BlendInt.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#ifndef _BLENDINT_ABSTRACTCAMERA_HPP_
#define _BLENDINT_ABSTRACTCAMERA_HPP_

#include <glm/glm.hpp>

#include <BlendInt/Object.hpp>

namespace BlendInt {

	class AbstractCamera: public Object
	{
	public:

		AbstractCamera ();

		AbstractCamera (const glm::vec3& pos, const glm::vec3& center, const glm::vec3& up);

		virtual ~AbstractCamera ();

		void LookAt (const glm::vec3& pos, const glm::vec3& center, const glm::vec3& up);

		/**
		 * @brief Set the projection data of this camera
		 * @param fovy
		 * @param aspect
		 * @param near
		 * @param far
		 */
		void SetPerspective (const float fovy, const float aspect,
		        const float near = 0.1f, const float far = 100.f);

		const glm::vec3& position () const {return m_position;}

		const glm::vec3& center () const {return m_center;}

		const glm::vec3& up () const {	return m_up;}

		const glm::vec3& local_x () const {return m_local_x;}

		const glm::vec3& local_y () const {return m_local_y;}

        const glm::vec3& local_z () const {return m_local_z;}
        
		const glm::mat4& projection () const	{return m_projection;}

		const glm::mat4& view () const {return m_view;}

		float fovy () const {return m_fovy;}

		float aspect () const	{return m_aspect;}

		float near () const	{return m_near;}

		float far () const {return m_far;}

		/**
		 * @brief Update the model view projection matrix
		 */
		virtual void Update () = 0;

	protected:

		void set_position (const glm::vec3& pos)
		{
			m_position = pos;
		}

		void set_position (float x, float y, float z)
		{
			m_position.x = x;
			m_position.y = y;
			m_position.z = z;
		}

		void set_center (const glm::vec3& center)
		{
			m_center = center;
		}

		void set_center (float x, float y, float z)
		{
			m_center = glm::vec3(x, y, z);
		}

		void set_up (const glm::vec3& up)
		{
			m_up = up;
		}

		void set_up (float x, float y, float z)
		{
			m_up = glm::vec3(x, y, z);
		}

		void set_local_x (const glm::vec3& x)
		{
			m_local_x = x;
		}

		void set_local_x (float x, float y, float z)
		{
			m_local_x.x = x;
			m_local_x.y = y;
			m_local_x.z = z;
		}

		void set_local_y (const glm::vec3& y)
		{
			m_local_y = y;
		}

		void set_local_y (float x, float y, float z)
		{
			m_local_y.x = x;
			m_local_y.y = y;
			m_local_y.z = z;
		}

        void set_local_z (const glm::vec3& z)
		{
			m_local_z = z;
		}
        
		void set_local_z (float x, float y, float z)
		{
			m_local_z.x = x;
			m_local_z.y = y;
			m_local_z.z = z;
		}

		void set_projection (const glm::mat4& projection)
		{
			m_projection = projection;
		}

		void set_view (const glm::mat4& view)
		{
			m_view = view;
		}

		void set_fovy (float fovy)
		{
			m_fovy = fovy;
		}

		void set_aspect (float aspect)
		{
			m_aspect = aspect;
		}

		void set_near (float near)
		{
			m_near = near;
		}

		void set_far (float far)
		{
			m_far = far;
		}

	private:

		/** The eye position */
		glm::vec3 m_position;

		/** Position of the reference point */
		glm::vec3 m_center;

		/** Direction of the up vector */
		glm::vec3 m_up;

		/** Right direction of the camera */
		glm::vec3 m_local_x;

		/** Up direction of the camera */
		glm::vec3 m_local_y;
        
        /** Look direction of the camera */
		glm::vec3 m_local_z;
        
		/** The view matrix */
		glm::mat4 m_view;

		/** The projection matrix */
		glm::mat4 m_projection;

		/** The field of view angle, in degrees, in the y direction */
		float m_fovy;

		/** Aspect ratio that determines the field of view in the x direction, the aspect ratio is the ratio of x (width) to y (height) */
		float m_aspect;

		/**	The distance from the viewer to the near clipping plane (always positive) */
		float m_near;

		/** The distance from the viewer to the far clipping plane (always positive) */
		float m_far;

	};

}

#endif /* _BLENDINT_ABSTRACTCAMERA_HPP_ */