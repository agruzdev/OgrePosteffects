
static const char Shader_Copy_V[] = ""
"#version 120                                                              \n"
"                                                                          \n"
"                                                                          \n"
"void main()                                                               \n"
"{                                                                         \n"
"    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
"}                                                                         \n"
"";

static const char Shader_Copy_F[] = ""
"#version 120                                                   \n"
"                                                               \n"
"uniform sampler2D texture;                                     \n"
"                                                               \n"
"void main()                                                    \n"
"{                                                              \n"
"    gl_FragColor = texture2D(texture, gl_TexCoord[0].st);      \n"
"}                                                              \n"
"";