/*
 * This file is part of BIL (Blender Interface Library).
 *
 * BIL (Blender Interface Library) is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BIL (Blender Interface Library) is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BIL.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#include <BIL/Button.hpp>
#include <iostream>

namespace BIL {

	const int WIDGET_AA_JITTER = 8;

	static const float jit[WIDGET_AA_JITTER][2] = {
		{ 0.468813, -0.481430}, {-0.155755, -0.352820},
		{ 0.219306, -0.238501}, {-0.393286, -0.110949},
		{-0.024699,  0.013908}, { 0.343805,  0.147431},
		{-0.272855,  0.269918}, { 0.095909,  0.388710}
	};

	GLubyte const checker_stipple_sml[32 * 32 / 8] =
		{
			255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
			255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
			0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
			0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
			255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
			255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
			0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
			0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
		};

	Button::Button (Widget* parent)
		: AbstractButton(parent)
	{

	}

	Button::Button (const wstring& text, Widget* parent)
		: AbstractButton(parent)
	{
		set_text (text);
	}

	Button::~Button ()
	{

	}

	void Button::Update ()
	{
		if (!size_.IsValid()) return;
		
		float rad;

		/* half rounded */
		// TODO: define widget_unit by user
		//rad = 0.2f * U.widget_unit;
		rad = 0.2f * 20;

		//round_box_edges(&wtb, roundboxalign, rect, rad);
		CalculateRoundBoxEdges (round_box_type_, Rect(pos_, size_), rad, &appearance_);
	}

	void Button::Render ()
	{
		DrawButton (&appearance_);

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_BLEND);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(pos_.x(),
					 pos_.y(),
					 z());

		text_.Render();

		glPopMatrix();
		//glDisable(GL_BLEND);
	}

	void Button::DrawButton (WidgetVertexes* vertexes)
	{
		Theme *theme = Theme::instance();
		if (theme == NULL) return;

		const WidgetColors* wcol = &(theme->themeUI()->wcol_tool);

		int j, a;	

		glEnable(GL_BLEND);

		/* backdrop non AA */
		if (vertexes->inner) {
			if (wcol->shaded == 0) {
				if (wcol->alpha_check) {
					float inner_v_half[WIDGET_SIZE_MAX][2];
					float x_mid = 0.0f; /* used for dumb clamping of values */

					/* dark checkers */
					glColor4ub(UI_TRANSP_DARK, UI_TRANSP_DARK, UI_TRANSP_DARK, 255);
					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(2, GL_FLOAT, 0, vertexes->inner_v);
					glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
					glDisableClientState(GL_VERTEX_ARRAY);

					/* light checkers */
					glEnable(GL_POLYGON_STIPPLE);
					glColor4ub(UI_TRANSP_LIGHT, UI_TRANSP_LIGHT, UI_TRANSP_LIGHT, 255);
					glPolygonStipple(checker_stipple_sml);

					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(2, GL_FLOAT, 0, vertexes->inner_v);
					glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
					glDisableClientState(GL_VERTEX_ARRAY);

					glDisable(GL_POLYGON_STIPPLE);

					/* alpha fill */
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					glColor4ub(wcol->inner.r(), wcol->inner.g(), wcol->inner.b(), wcol->inner.a());

					if (hover_) {
						glColor4ub(wcol->inner_highlight.r(), wcol->inner_highlight.g(),
								   wcol->inner_highlight.b(), wcol->inner_highlight.a());
					}

					if (down_) {
						glColor4ub(wcol->inner_sel.r(), wcol->inner_sel.g(),
								   wcol->inner_sel.b(), wcol->inner_sel.a());
					}

					glEnableClientState(GL_VERTEX_ARRAY);

					for (a = 0; a < vertexes->totvert; a++) {
						x_mid += vertexes->inner_v[a][0];
					}
					x_mid /= vertexes->totvert;

					glVertexPointer(2, GL_FLOAT, 0, vertexes->inner_v);
					glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
					glDisableClientState(GL_VERTEX_ARRAY);

					/* 1/2 solid color */
				
					glColor4ub(wcol->inner.r(), wcol->inner.g(),
							   wcol->inner.b(), 255);

					if (hover_) {
						glColor4ub(wcol->inner_highlight.r(), wcol->inner_highlight.g(),
								   wcol->inner_highlight.b(), 255);
					}
					
					if (down_) {
						glColor4ub(wcol->inner_sel.r(), wcol->inner_sel.g(),
								   wcol->inner_sel.b(), 255);
					}

					for (a = 0; a < vertexes->totvert; a++) {
						inner_v_half[a][0] = std::min(vertexes->inner_v[a][0], x_mid);
						inner_v_half[a][1] = vertexes->inner_v[a][1];
					}

					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(2, GL_FLOAT, 0, inner_v_half);
					glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
					glDisableClientState(GL_VERTEX_ARRAY);
				}
				else {
					/* simple fill */
					glColor4ub(wcol->inner.r(), wcol->inner.g(), wcol->inner.b(), wcol->inner.a());

					if (hover_) {
						glColor4ub(wcol->inner_highlight.r(), wcol->inner_highlight.g(),
								   wcol->inner_highlight.b(), wcol->inner_highlight.a());
					}
					
					if (down_) {
						glColor4ub(wcol->inner_sel.r(), wcol->inner_sel.g(),
								   wcol->inner_sel.b(), wcol->inner_sel.a());
					}

					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(2, GL_FLOAT, 0, vertexes->inner_v);
					glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
					glDisableClientState(GL_VERTEX_ARRAY);
				}
			}
			else {
				//char col1[4], col2[4];
				Color col1, col2;
				unsigned char col_array[WIDGET_SIZE_MAX * 4];
				unsigned char *col_pt = col_array;
			
				Color::ConvertShadeColor(wcol->inner, wcol->shadetop, wcol->shadedown, &col1, &col2);

				glShadeModel(GL_SMOOTH);
				for (a = 0; a < vertexes->totvert; a++, col_pt += 4) {
					Color::ConvertRoundBoxShadeColor(col1, col2, vertexes->inner_uv[a][vertexes->shadedir], col_pt);
				}

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				glVertexPointer(2, GL_FLOAT, 0, vertexes->inner_v);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, col_array);
				glDrawArrays(GL_POLYGON, 0, vertexes->totvert);
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);

				glShadeModel(GL_FLAT);
			}
		}
	
		/* for each AA step */
		if (vertexes->outline) {
			float quad_strip[WIDGET_SIZE_MAX * 2 + 2][2]; /* + 2 because the last pair is wrapped */
			float quad_strip_emboss[WIDGET_SIZE_MAX * 2][2]; /* only for emboss */

			const unsigned char tcol[4] = {wcol->outline[0],
										   wcol->outline[1],
										   wcol->outline[2],
										   wcol->outline[3] / WIDGET_AA_JITTER};

			verts_to_quad_strip(vertexes->totvert, quad_strip, vertexes);

			if (vertexes->emboss) {
				verts_to_quad_strip_open(vertexes->halfwayvert, quad_strip_emboss, vertexes);
			}

			glEnableClientState(GL_VERTEX_ARRAY);

			for (j = 0; j < WIDGET_AA_JITTER; j++) {
				glTranslatef(1.0f * jit[j][0], 1.0f * jit[j][1], 0.0f);
			
				/* outline */
				glColor4ubv(tcol);

				glVertexPointer(2, GL_FLOAT, 0, quad_strip);
				glDrawArrays(GL_QUAD_STRIP, 0, vertexes->totvert * 2 + 2);
		
				/* emboss bottom shadow */
				if (vertexes->emboss) {
					glColor4f(1.0f, 1.0f, 1.0f, 0.02f);

					glVertexPointer(2, GL_FLOAT, 0, quad_strip_emboss);
					glDrawArrays(GL_QUAD_STRIP, 0, vertexes->halfwayvert * 2);
				}
			
				glTranslatef(-1.0f * jit[j][0], -1.0f * jit[j][1], 0.0f);
			}

			glDisableClientState(GL_VERTEX_ARRAY);
		}
	
		/* decoration */
		if (vertexes->tria1.tot || vertexes->tria2.tot) {
			const unsigned char tcol[4] = {wcol->item[0],
										   wcol->item[1],
										   wcol->item[2],
										   (unsigned char)((float)wcol->item[3] / WIDGET_AA_JITTER)};
			/* for each AA step */
			for (j = 0; j < WIDGET_AA_JITTER; j++) {
				glTranslatef(1.0f * jit[j][0], 1.0f * jit[j][1], 0.0f);

				if (vertexes->tria1.tot) {
					glColor4ubv(tcol);
					DrawTrias(&vertexes->tria1);
				}
				if (vertexes->tria2.tot) {
					glColor4ubv(tcol);
					DrawTrias(&vertexes->tria2);
				}
		
				glTranslatef(-1.0f * jit[j][0], -1.0f * jit[j][1], 0.0f);
			}
		}

		glDisable(GL_BLEND);
	}

}
