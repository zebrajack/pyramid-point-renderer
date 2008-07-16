/*
** pyramid_point_renderer.cc Pyramid Point Based Rendering.
**
**
**   history:	created  02-Jul-07
*/

#include "pyramid_point_renderer_color.h"

/**
 * Default constructor.
 **/
PyramidPointRendererColor::PyramidPointRendererColor() : PointBasedRenderer(),
						     fbo_width(1800),
						     fbo_height(1200),
						     fbo_buffers_count(6),
						     canvas_border_width(32),
						     canvas_border_height(32),
						     render_state(RS_BUFFER0) {
  createFBO();
  createShaders();
  levels_count = MAX((int)(log(fbo_width)/log(2.0)), (int)(log(fbo_height)/log(2.0)));
}

PyramidPointRendererColor::PyramidPointRendererColor(int w, int h) : PointBasedRenderer(w, h),
								 fbo_width(1800),
								 fbo_height(1200),
								 fbo_buffers_count(6),
								 canvas_border_width(w/32),
								 canvas_border_height(h/32),
								 render_state(RS_BUFFER0) {
  createFBO();
  createShaders();
  levels_count = MAX((int)(log(canvas_width)/log(2.0)), (int)(log(canvas_height)/log(2.0)));
}

PyramidPointRendererColor::~PyramidPointRendererColor() {
  delete shader_projection;
  delete shader_analysis;
  delete shader_copy;
  delete shader_synthesis;
  delete shader_phong;
  delete shader_show;

  glDeleteTextures(FBO_BUFFERS_COUNT_6, fbo_textures);
  for (int i = 0; i < FBO_BUFFERS_COUNT_6; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(FBO_TYPE, 0);
    glDisable(FBO_TYPE);
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
}


GLuint PyramidPointRendererColor::getTextureOfBuffer(GLuint buffer)
     /* returns the OpenGL texture name of a color attachment buffer (GL_COLOR_ATTACHMENTx_EXT) 
      * of the global framebuffer object or 0 if the color attachment buffer is not bound
      */
{
  int i;

  for (i = 0; i < FBO_BUFFERS_COUNT_6; i++)
    {

      if (fbo_buffers[i] == buffer)
	{
	  break;
	}
    }
  if (i < FBO_BUFFERS_COUNT_6)
    {
      return fbo_textures[i];
    }
  else
    {
      return (GLuint)0;
    }
}

void PyramidPointRendererColor::rasterizePixels(pixels_struct dest, pixels_struct src0, pixels_struct src1,
		     int phase)
     /* binds buffers and rasterizes quad; calls callbackfunc to set the fragment program */
{
  /* bind framebuffer and renderbuffers */


  if (0 == dest.fbo)
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      glDrawBuffer(dest.buffers[0]);

      if (framebuffer_state != FBS_SYSTEM_PROVIDED)
	{
	  glViewport(0, 0, canvas_width + 2*canvas_border_width, 
		     canvas_height + 2*canvas_border_height);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  gluOrtho2D(0.0, canvas_width + 2*canvas_border_width, 
		     0.0, canvas_height + 2*canvas_border_height);

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  framebuffer_state = FBS_SYSTEM_PROVIDED;
	}
    }
  else
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, dest.fbo);
      glDrawBuffers(dest.buffersCount, dest.buffers);

      if (framebuffer_state != FBS_APPLICATION_CREATED) 
	{
	  glViewport(0, 0, fbo_width, fbo_height);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  gluOrtho2D(0.0, fbo_width, 0.0, fbo_height);
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  framebuffer_state = FBS_APPLICATION_CREATED;
	}
    }

  /* bind textures: src0 is bound as 0th and 1st texture, src1 as 2nd and 3rd texture */
  {
    if (src0.buffersCount > 0)
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(FBO_TYPE, src0.textures[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }

    if (src0.buffersCount > 1)
      {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(FBO_TYPE, src0.textures[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }

    if (src0.buffersCount > 2)
      {
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(FBO_TYPE, src0.textures[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }

    if (src1.buffersCount > 0)
      {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(FBO_TYPE, src1.textures[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }

    if (src1.buffersCount > 1)
      {
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(FBO_TYPE, src1.textures[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }

    if (src1.buffersCount > 2)
      {
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(FBO_TYPE, src1.textures[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    else
      {
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(FBO_TYPE, (GLuint)0);
      }
  }

  if (EMPTY != phase)
    {
      bool ret = TRUE;
      switch(phase) {
      case PROJECTION:
	ret = projectionCallbackFunc();	
	break;
      case ANALYSIS:
	ret = analysisCallbackFunc();
	break;
      case COPY:
	ret = copyCallbackFunc();
	break;
      case SYNTHESIS:
	ret = synthesisCallbackFunc();
	break;
      case PHONG:
	ret = phongShadingCallbackFunc();
	break;
      case SHOW:
	ret = showCallbackFunc();
	break;
      }
      if (FALSE != ret)
	{
	  return; /* callback function has done the rendering */
	}
    }

  /* set vertex and texture coordinates and rasterize */
  {
    GLfloat texcoors0[4][2];
    GLfloat texcoors1[4][2];
    GLfloat vertices[4][2];
  
    int i;
	  
    vertices[0][0] = (dest.x); 
    vertices[0][1] = (dest.y);
    vertices[1][0] = (dest.x); 
    vertices[1][1] = (dest.y + dest.height);
    vertices[2][0] = (dest.x + dest.width); 
    vertices[2][1] = (dest.y + dest.height);
    vertices[3][0] = (dest.x + dest.width); 
    vertices[3][1] = (dest.y);

    texcoors0[0][0] = (src0.x) / (GLfloat)fbo_width; 
    texcoors0[0][1] = (src0.y) / (GLfloat)fbo_height;
    texcoors0[1][0] = (src0.x) / (GLfloat)fbo_width; 
    texcoors0[1][1] = (src0.y + src0.height) / (GLfloat)fbo_height;
    texcoors0[2][0] = (src0.x + src0.width) / (GLfloat)fbo_width; 
    texcoors0[2][1] = (src0.y + src0.height) / (GLfloat)fbo_height;
    texcoors0[3][0] = (src0.x + src0.width) / (GLfloat)fbo_width; 
    texcoors0[3][1] = (src0.y) / (GLfloat)fbo_height;

    texcoors1[0][0] = (src1.x) / (GLfloat)fbo_width; 
    texcoors1[0][1] = (src1.y) / (GLfloat)fbo_height;
    texcoors1[1][0] = (src1.x) / (GLfloat)fbo_width; 
    texcoors1[1][1] = (src1.y + src1.height) / (GLfloat)fbo_height;
    texcoors1[2][0] = (src1.x + src1.width) / (GLfloat)fbo_width; 
    texcoors1[2][1] = (src1.y + src1.height) / (GLfloat)fbo_height;
    texcoors1[3][0] = (src1.x + src1.width) / (GLfloat)fbo_width; 
    texcoors1[3][1] = (src1.y) / (GLfloat)fbo_height;


    /* send quad */
    
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    for(i = 0; i < 4; i++) 
      {	
	if (src0.buffersCount > 0)
	  glMultiTexCoord2fARB(GL_TEXTURE0, texcoors0[i][0], texcoors0[i][1]);
	if (src0.buffersCount > 1)
	  glMultiTexCoord2fARB(GL_TEXTURE1, texcoors0[i][0], texcoors0[i][1]);
	if (src0.buffersCount > 2)
	  glMultiTexCoord2fARB(GL_TEXTURE2, texcoors0[i][0], texcoors0[i][1]);

	if (src1.buffersCount > 0)
	  glMultiTexCoord2fARB(GL_TEXTURE3, texcoors1[i][0], texcoors1[i][1]);
	if (src1.buffersCount > 1)
	  glMultiTexCoord2fARB(GL_TEXTURE4, texcoors1[i][0], texcoors1[i][1]);
	if (src1.buffersCount > 2)
	  glMultiTexCoord2fARB(GL_TEXTURE5, texcoors1[i][0], texcoors1[i][1]);

	glVertex2f(vertices[i][0], vertices[i][1]);
      }
    glEnd();
  }
}

pixels_struct PyramidPointRendererColor::generatePixels(int level, 
			     GLuint curr_fbo, int buffersCount, GLuint buffer0, GLuint buffer1, GLuint buffer2)
     /* returns an pixels_struct specifying the pixels of the specified level in the
      * specified fbo (0 or g.fbo) and one or two of its buffers (GL_BACK or g.fbo_buffers[i])
      */
{
  pixels_struct result;

  result.width = (canvas_width >> level);
  if (result.width < 1)
    result.width = 1;
  result.height = (canvas_height >> level);
  if (result.height < 1)
    result.height = 1;
  if (0 == level)
    {
      result.x = canvas_border_width;
      result.y = canvas_border_height;
    }
  else if (1 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + canvas_border_width / 2;
      result.y = canvas_border_height / 2;
    } 
  else if (2 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + canvas_border_width / 2;
      result.y = (canvas_height + 2 * canvas_border_height) / 2 + canvas_border_height / 4;
    }
  else if (3 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + 
	(canvas_width + 2 * canvas_border_width) / 16;
      result.y = (canvas_height + 2 * canvas_border_height) / 2 +
	(canvas_height + 2 * canvas_border_height) / 4 +
	canvas_border_height / 4;
    }
  else
    {
      int i;

      result.x = (canvas_width + 2 * canvas_border_width) + 
	(canvas_width + 2 * canvas_border_width) / 4 +
	(canvas_width + 2 * canvas_border_width) / 8 -
	((canvas_width + 2 * canvas_border_width) >> (level + 1));
      result.y = (canvas_height + 2 * canvas_border_height) / 2;
      for (i = 4; i < level; i++)
	{
	  result.y += ((canvas_height + 2 * canvas_border_height) >> i) + 2;
	}
      result.y += (canvas_border_height >> level) + 2; 
    }

  result.fbo = curr_fbo;
  result.buffersCount = buffersCount;
  result.buffers[0] = buffer0;
  result.textures[0] = getTextureOfBuffer(buffer0);
  result.buffers[1] = buffer1;
  result.textures[1] = getTextureOfBuffer(buffer1);
  result.buffers[2] = buffer2;
  result.textures[2] = getTextureOfBuffer(buffer2);

  return result;
}


int PyramidPointRendererColor::projectionCallbackFunc( void )
{
  shader_projection->use();
  shader_projection->set_uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);
  shader_projection->set_uniform("back_face_culling", (GLint)back_face_culling);

  if (use_lod == 1) { //lod
    shader_projection->set_uniform("vertex_buffer", 6);
    shader_projection->set_uniform("normal_buffer", 7);
    shader_projection->set_uniform("color_per_lod", (GLint)color_per_lod);
  }
  else if (use_lod == 2) { //upsampling
    GLfloat max_radius = 2.0/(GLfloat)canvas_width;
    shader_projection->set_uniform("max_radius", max_radius);
  }

  return TRUE;
}

/// Project point sized samples to screen space
void PyramidPointRendererColor::projectSurfels( Primitives* prim )
{
  pixels_struct nullPixels;
  pixels_struct destinationPixels;

  if (use_lod)
    num_primitives = prim->numPrimitivesLOD();

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  framebuffer_state = FBS_APPLICATION_CREATED;

  destinationPixels = generatePixels(0, fbo, 3, fbo_buffers[0], fbo_buffers[2], fbo_buffers[4]);
  rasterizePixels(destinationPixels, nullPixels, nullPixels, PROJECTION);

  // Render vertices using the vertex buffer object.
  glPointSize(1.0);

  prim->render();

  shader_projection->use(0);
}


double PyramidPointRendererColor::computeHalfPixelSize( void ) {

  double d = pow(2.0, (double)cur_level) / (double)(canvas_width);
  d *= 0.5;

  return d;
}

int PyramidPointRendererColor::analysisCallbackFunc( void )
{
  shader_analysis->use();
  shader_analysis->set_uniform("oo_2fbo_size", (GLfloat)(0.5 / fbo_width), (GLfloat)(0.5 / fbo_height));

  shader_analysis->set_uniform("half_pixel_size", (GLfloat)computeHalfPixelSize());
  shader_analysis->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_analysis->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));

  shader_analysis->set_uniform("depth_test", depth_test);

  shader_analysis->set_uniform("textureA", 0);
  shader_analysis->set_uniform("textureB", 1);
  shader_analysis->set_uniform("textureC", 2);

  return FALSE; /* not done, rasterize quad */
}

void PyramidPointRendererColor::rasterizeAnalysisPyramid( void )
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  int level;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  for (level = 1; level < levels_count; level++)
    {
      cur_level = level - 1;
      sourcePixels = generatePixels(level - 1, fbo, 3,
				    fbo_buffers[0 + ((level - 1) % 2)], 
				    fbo_buffers[2 + ((level - 1) % 2)],
      				    fbo_buffers[4 + ((level - 1) % 2)]);
      destinationPixels = generatePixels(level, fbo, 3,
					 fbo_buffers[0 + (level % 2)], 
					 fbo_buffers[2 + (level % 2)],
      					 fbo_buffers[4 + (level % 2)]);
      rasterizePixels(destinationPixels, sourcePixels, nullPixels, ANALYSIS);
      shader_analysis->use(0);
    }
}

int PyramidPointRendererColor::copyCallbackFunc( void )
{
  shader_copy->use();
  shader_copy->set_uniform("textureA", 0);
  shader_copy->set_uniform("textureB", 1);
  shader_copy->set_uniform("textureC", 2);

  return FALSE; /* not done, rasterize quad */
}

void PyramidPointRendererColor::copyAnalysisPyramid()
     /* copies odd levels from color attachment pair 1-3 to buffer pair 0-2 and 
      * even levels from 0-2 to 1-3.
      */
{
  int level;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  for (level = 0; level < levels_count; level++)
    {
      sourcePixels = generatePixels(level, fbo, 3,
				    fbo_buffers[0 + (level % 2)], 
				    fbo_buffers[2 + (level % 2)],
      				    fbo_buffers[4 + (level % 2)]);
      destinationPixels = generatePixels(level, fbo, 3,
					 fbo_buffers[0 + ((level + 1) % 2)], 
					 fbo_buffers[2 + ((level + 1) % 2)],
      					 fbo_buffers[4 + ((level + 1) % 2)]);
      rasterizePixels(destinationPixels, sourcePixels, nullPixels, COPY);
      shader_copy->use(0);
    }
}

int PyramidPointRendererColor::synthesisCallbackFunc( void )
{
  shader_synthesis->use();
  shader_synthesis->set_uniform("fbo_size", (GLfloat)fbo_width, (GLfloat)fbo_height);
  shader_synthesis->set_uniform("oo_fbo_size", (GLfloat)(1.0/fbo_width), (GLfloat)(1.0/fbo_height));

  shader_synthesis->set_uniform("half_pixel_size", (GLfloat)computeHalfPixelSize());
  shader_synthesis->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_synthesis->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));

  shader_synthesis->set_uniform("depth_test", depth_test);
  shader_synthesis->set_uniform("elliptical_weight", elliptical_weight);

  shader_synthesis->set_uniform("textureA", 0);
  shader_synthesis->set_uniform("textureB", 1);
  shader_synthesis->set_uniform("textureC", 2);

  return FALSE; /* not done, rasterize quad */
}

void PyramidPointRendererColor::rasterizeSynthesisPyramid()
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  int level;
  pixels_struct source0Pixels; /* same level as destination */
  pixels_struct source1Pixels; /* coarser level than destination */
  pixels_struct destinationPixels;

  for (level = levels_count - 2; level >= 0; level--)
    {
      cur_level = level;

      source0Pixels = generatePixels(level, fbo, 3,
				     fbo_buffers[0 + ((level + 1) % 2)],
				     fbo_buffers[2 + ((level + 1) % 2)],
      				     fbo_buffers[4 + ((level + 1) % 2)]);
      source1Pixels = generatePixels(level + 1, fbo, 3,
				     fbo_buffers[0 + ((level + 1) % 2)],
				     fbo_buffers[2 + ((level + 1) % 2)],
      				     fbo_buffers[4 + ((level + 1) % 2)]);
      destinationPixels = generatePixels(level, fbo, 3,
					 fbo_buffers[0 + (level % 2)],
					 fbo_buffers[2 + (level % 2)],
     					 fbo_buffers[4 + (level % 2)]);
      rasterizePixels(destinationPixels, source0Pixels, source1Pixels, SYNTHESIS);
      shader_synthesis->use(0);
    }
}

/* rasterize level 0 of pyramid with per pixel shading */

int PyramidPointRendererColor::phongShadingCallbackFunc( void )
{
  shader_phong->use();
  shader_phong->set_uniform("textureA", 0);
  shader_phong->set_uniform("textureC", 1);
  shader_phong->set_uniform("normalBuffer", normal_buffer);

//   if (use_lod)
//     shader_phong->set_uniform("color_per_lod", (GLint)color_per_lod);
//   else
//     shader_phong->set_uniform("color_per_lod", (GLint)0);

  return FALSE; /* not done, rasterize quad */
}

void PyramidPointRendererColor::rasterizePhongShading(int bufferIndex)
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  int level = 0;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  shader_projection->use(0);
  shader_analysis->use(0);
  shader_copy->use(0);
  shader_synthesis->use(0);
  shader_show->use(0);

  sourcePixels = generatePixels(level, fbo, 2, fbo_buffers[bufferIndex], fbo_buffers[bufferIndex + 4], 0);  
  destinationPixels = generatePixels(level, 0, 1, GL_BACK, 0, 0);
  rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

  shader_phong->use(0);
}

/* rasterize level 0 of pyramid with per pixel shading */

int PyramidPointRendererColor::showCallbackFunc( void )
{
  shader_show->use();
  shader_show->set_uniform("tex", 0);

  return FALSE; /* not done, rasterize quad */
}

/* show buffer */
void PyramidPointRendererColor::showPixels(int bufferIndex)
{
  int level = 0;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  shader_projection->use(0);
  shader_analysis->use(0);
  shader_copy->use(0);
  shader_synthesis->use(0);
  shader_phong->use(0);

  for (level = 0; level < levels_count; level++)
    {
      sourcePixels = generatePixels(level, fbo, 1, fbo_buffers[bufferIndex], 0, 0);
      destinationPixels = generatePixels(level, 0, 1, GL_BACK, 0, 0);
      rasterizePixels(destinationPixels, sourcePixels, nullPixels, SHOW);
      shader_show->use(0);
    }
}

/**
 * Clear all framebuffers and screen buffer.
 **/
void PyramidPointRendererColor::clearBuffers() {
  int i;

  glEnable(FBO_TYPE);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
  
  GLint currentDrawBuffer;
  glGetIntegerv(GL_DRAW_BUFFER, &currentDrawBuffer);

  // Clear all buffers, including the render buffer
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  for (i = 0; i < FBO_BUFFERS_COUNT_6; i++) {
    glDrawBuffer(fbo_buffers[i]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //glClear(GL_COLOR_BUFFER_BIT); 
  framebuffer_state = FBS_UNDEFINED;

  CHECK_FOR_OGL_ERROR();
}

/**
 * Reconstructs the surface for visualization.
 **/
void PyramidPointRendererColor::projectSamples(Primitives* prim) {
  // Project points to framebuffer with depth test on.

  projectSurfels( prim );

  CHECK_FOR_OGL_ERROR();
}


/**
 * Interpolate projected samples using pyramid interpolation
 * algorithm.
 **/
void PyramidPointRendererColor::interpolate() {
  framebuffer_state = FBS_UNDEFINED;

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Interpolate scattered data using pyramid algorithm

  rasterizeAnalysisPyramid();

  copyAnalysisPyramid();

  rasterizeSynthesisPyramid();
}

/**
 * Render final image with normal map to a given buffer instead of screen.
 * @param data Given buffer to write data to.
 * @param w Width of buffer.
 * @param h Height of buffer.
 **/
void PyramidPointRendererColor::drawNormalsToBuffer ( GLfloat* data, int w, int h ) {
  int level = 0;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0, 0, 0);

  shader_projection->use(0);
  shader_analysis->use(0);
  shader_copy->use(0);
  shader_synthesis->use(0);
  shader_show->use(0);
  normal_buffer = 1;

  sourcePixels = generatePixels(level, fbo, 2, fbo_buffers[0], fbo_buffers[4], 0);
  destinationPixels = generatePixels(level, fbo, 1, fbo_buffers[1], 0, 0);
  rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

  shader_phong->use(0);

  glReadBuffer(fbo_buffers[1]);
  glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, &data[0]);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
}

/**
 * Renders reconstructed model on screen with
 * per pixel shading.
 **/
void PyramidPointRendererColor::draw( void ) {

  // Deffered shading of the final image containing normal map
  normal_buffer = 0;
  rasterizePhongShading(0);
  //showPixels(1);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);

  glDisable(FBO_TYPE);

  CHECK_FOR_OGL_ERROR();
}


/**
 * Initialize OpenGL state variables.
 **/
void PyramidPointRendererColor::createFBO() {
  int i;
  GLenum framebuffer_status;
  GLenum attachments[16] = {
    GL_COLOR_ATTACHMENT0_EXT,
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT,
    GL_COLOR_ATTACHMENT4_EXT,
    GL_COLOR_ATTACHMENT5_EXT,
    GL_COLOR_ATTACHMENT6_EXT,
    GL_COLOR_ATTACHMENT7_EXT,
    GL_COLOR_ATTACHMENT8_EXT,
    GL_COLOR_ATTACHMENT9_EXT,
    GL_COLOR_ATTACHMENT10_EXT,
    GL_COLOR_ATTACHMENT11_EXT,
    GL_COLOR_ATTACHMENT12_EXT,
    GL_COLOR_ATTACHMENT13_EXT,
    GL_COLOR_ATTACHMENT14_EXT,
    GL_COLOR_ATTACHMENT15_EXT
  };

  /* initialize fbos */

  assert(FBO_BUFFERS_COUNT_6 <= 16);

  glGenTextures(FBO_BUFFERS_COUNT_6, fbo_textures);
  for (i = 0; i < FBO_BUFFERS_COUNT_6; i++) {
    fbo_buffers[i] = attachments[i];
    glBindTexture(FBO_TYPE, fbo_textures[i]);
    glTexImage2D(FBO_TYPE, 0, FBO_FORMAT,
		 fbo_width, fbo_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  //for creating and binding a depth buffer:
  glGenTextures(1, &fbo_depth);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, fbo_width,
			   fbo_height);

  CHECK_FOR_OGL_ERROR();

  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  CHECK_FOR_OGL_ERROR();

  for (i = 0; i < FBO_BUFFERS_COUNT_6; i++) 
    {
      //      fprintf(stderr, "bind fbo buffer %i\n", i);
      glBindTexture(FBO_TYPE, fbo_textures[i]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				fbo_buffers[i], FBO_TYPE, fbo_textures[i], 0);
      CHECK_FOR_OGL_ERROR();
    }

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			       GL_RENDERBUFFER_EXT, fbo_depth);
 
  CHECK_FOR_OGL_ERROR();

  framebuffer_status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(framebuffer_status) 
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      fprintf(stderr, "Framebuffer not supported\n");
      exit(1);
      break;
    default:
      fprintf(stderr, "Framebuffer error %x", framebuffer_status);
      exit(1);
      return ;
    }
 
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  framebuffer_state = FBS_UNDEFINED;

  glShadeModel(GL_FLAT);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  CHECK_FOR_OGL_ERROR();

  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &i);
  //  fprintf(stderr, "max color attachments %i\n", i);
}		

/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PyramidPointRendererColor::createShaders ( void ) {

  bool shader_inst_debug = 1;

  shader_projection_no_lod = new glslKernel();
  shader_projection_no_lod->vertex_source("pyramid_point_renderer_color/shader_point_projection_color.vert");
  shader_projection_no_lod->fragment_source("pyramid_point_renderer_color/shader_point_projection_color.frag");
  shader_projection_no_lod->install( shader_inst_debug );

  shader_projection_upsampling = new glslKernel();
  shader_projection_upsampling->vertex_source("pyramid_point_renderer_color/shader_point_projection_upsampling.vert");
  shader_projection_upsampling->geometry_source("pyramid_point_renderer_color/shader_point_projection_upsampling.geom");
  shader_projection_upsampling->set_geom_max_output_vertices( 9 );
  shader_projection_upsampling->set_geom_input_type(GL_POINTS);
  shader_projection_upsampling->set_geom_output_type(GL_POINTS);
  shader_projection_upsampling->fragment_source("pyramid_point_renderer_color/shader_point_projection_upsampling.frag");
  shader_projection_upsampling->install( shader_inst_debug );

  shader_projection_lod = new glslKernel();
  shader_projection_lod->vertex_source("pyramid_point_renderer_color/shader_point_projection_lod.vert");
  shader_projection_lod->geometry_source("pyramid_point_renderer_color/shader_point_projection_lod.geom");
  shader_projection_lod->set_geom_max_output_vertices( (int)pow((double)MAX_LEAF_SURFELS, (double)LOD_LEVELS-1) );
  shader_projection_lod->set_geom_input_type(GL_POINTS);
  shader_projection_lod->set_geom_output_type(GL_POINTS);
  shader_projection_lod->fragment_source("pyramid_point_renderer_color/shader_point_projection_color.frag");
  shader_projection_lod->install( shader_inst_debug );  

  shader_projection = shader_projection_no_lod;

  shader_analysis = new glslKernel();
  shader_analysis->vertex_source("pyramid_point_renderer_color/shader_analysis_color.vert");
  shader_analysis->fragment_source("pyramid_point_renderer_color/shader_analysis_color.frag");
  shader_analysis->install( shader_inst_debug );

  shader_copy = new glslKernel();
  shader_copy->vertex_source("pyramid_point_renderer_color/shader_copy_color.vert");
  shader_copy->fragment_source("pyramid_point_renderer_color/shader_copy_color.frag");
  shader_copy->install( shader_inst_debug );

  shader_synthesis = new glslKernel();
  shader_synthesis->vertex_source("pyramid_point_renderer_color/shader_synthesis_color.vert");
  shader_synthesis->fragment_source("pyramid_point_renderer_color/shader_synthesis_color.frag");
  shader_synthesis->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("pyramid_point_renderer_color/shader_phong_color.vert");
  shader_phong->fragment_source("pyramid_point_renderer_color/shader_phong_color.frag");
  shader_phong->install( shader_inst_debug );

  shader_show = new glslKernel();
  shader_show->vertex_source("pyramid_point_renderer_color/shader_show.vert");
  shader_show->fragment_source("pyramid_point_renderer_color/shader_show.frag");
  shader_show->install( shader_inst_debug );
}


