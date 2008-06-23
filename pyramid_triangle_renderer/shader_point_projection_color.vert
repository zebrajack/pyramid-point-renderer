// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
uniform int back_face_culling;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

void main(void)
{
  
  vec3 e = (gl_ModelViewProjectionMatrixInverse * vec4(eye, 1.0)).xyz;
  float material = 0.0;

  gl_TexCoord[0] = gl_MultiTexCoord0;
  vec3 unprojected_normal = cross(normalize(gl_TexCoord[0].xyz), normalize(gl_Normal.xyz));

  //if ( (back_face_culling == 1) && (dot(normalize(gl_Vertex.xyz - eye), gl_Normal) < 0.0 )) {
  if ( (back_face_culling == 1) && (dot(normalize(e - gl_Vertex.xyz), unprojected_normal) < 0.0 )) {

    radius_depth_w.x = 0.0;
    
    // for some reason seting the vector to vec4(0.0) drops
    // the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    gl_Position = vec4(1.0);
  }
  else
    {
      // only rotate point and normal if not culled
      vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
      
      material = gl_TexCoord[0].w;

      normal_vec = normalize(gl_NormalMatrix * unprojected_normal);
      float radius = gl_Vertex.w;

      // the projected normal is the smaller projection between the minor and major axis
      // compute first the projected axis
/*       vec3 minor_axis = normalize(gl_NormalMatrix * gl_TexCoord[0].xyz); */
/*       vec3 major_axis = normalize(gl_NormalMatrix * gl_Normal.xyz); */
/* /\*       float minor_len = length(gl_TexCoord[0].xyz); *\/ */
/* /\*       float major_len = length(gl_Normal.xyz); *\/ */
/*       float minor_len = length(gl_NormalMatrix * gl_TexCoord[0].xyz); */
/*       float major_len = length(gl_NormalMatrix * gl_Normal.xyz); */

/*       vec3 minor_axis = normalize(gl_NormalMatrix * gl_TexCoord[0].xyz); */
/*       vec3 major_axis = normalize(gl_NormalMatrix * gl_Normal.xyz); */
/*       float minor_len = length(gl_NormalMatrix * gl_TexCoord[0].xyz); */
/*       float major_len = length(gl_NormalMatrix * gl_Normal.xyz); */

/* /\*       float n_factor = 1.0 / sqrt(minor_axis.x*minor_axis.x + minor_axis.y*minor_axis.y); *\/ */
/* /\*       normal_vec.x *= n_factor; *\/ */
/* /\*       normal_vec.y *= n_factor; *\/ */
/*       normal_vec.xy = normalize(minor_axis.xy); */
/*       normal_vec.z = minor_len / major_len; */

/*       float radius = major_len; */
 
      //      if (minor_axis.z * minor_len > major_axis.z * major_len) {
/*       if (length((gl_NormalMatrix * gl_TexCoord[0].xyz).xy) > length((gl_NormalMatrix * gl_Normal.xyz).xy)) { */
/* 	radius = minor_len; */
/* 	normal_vec = vec3(major_axis.xy, major_len / minor_len); */
/*       } */
/*       else { */
/* 	radius = major_len; */
/* 	normal_vec = vec3(minor_axis.xy, minor_len / major_len); */
/*       } */

/*       if (minor_len < major_len) { */
/* 	radius = major_len; */
/* 	normal_vec = minor_axis; */
/* 	normal_vec.z = minor_len / major_len;	 */
/*       } */
/*       else { */
/* 	radius = minor_len; */
/* 	normal_vec = major_axis; */
/* 	normal_vec.z = major_len / minor_len; */
/*       } */
/*       normal_vec.xy = normalize(normal_vec.xy); */

/*       normal_vec.xy *= (1.0 - normal_vec.z*normal_vec.z) / (normal_vec.x*normal_vec.x + normal_vec.y*normal_vec.y); */

      // compute depth value without projection matrix, only modelview
      radius_depth_w = vec3(radius, -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z, v.w);
      
      gl_Position = v;
    }
  gl_FrontColor = vec4(0.0, 0.0, 0.0, material);
}
