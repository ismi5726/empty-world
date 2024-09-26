#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

// adapted from https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h
void shaderErrorCheck(unsigned int shader, char *type) {
    int success;
    char infoLog[1024];
    if (*type != 'P') {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("shader compilation error of type %s\n%s\n*************************",
                type, infoLog
            );
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("program linking error of type %s\n%s\n*************************",
                type, infoLog
            );
        }
    }
}

unsigned int createProgram(char *vertSource, char *fragSource, char isFile) {
    char *vertProg, *fragProg;
    if (isFile) {
        // file reading code from https://stackoverflow.com/a/3747128
        // (i am not a very good c programmer, sorry)
        FILE *fp;
        long lSize;

        // vertex
        fp = fopen(vertSource, "rb");
        if (!fp) {
            perror(vertSource);
            exit(1);
        }

        fseek(fp, 0L, SEEK_END);
        lSize = ftell(fp);
        rewind(fp);

        vertProg = calloc(1, lSize + 1);
        if(!vertProg) {
            fclose(fp);
            fputs("memory alloc fails",stderr);
            exit(1);
        }

        /* copy the file into the buffer */
        if(1!=fread(vertProg , lSize, 1 , fp)) {
            fclose(fp);
            free(vertProg);
            fputs("entire read fails",stderr);
            exit(1);
        }

        // fragment
        fp = fopen(fragSource, "rb");
        if (!fp) {
            perror(fragSource);
            exit(1);
        }

        fseek(fp, 0L, SEEK_END);
        lSize = ftell(fp);
        rewind(fp);

        fragProg = calloc(1, lSize + 1);
        if(!fragProg) {
            fclose(fp);
            fputs("memory alloc fails",stderr);
            exit(1);
        }

        /* copy the file into the buffer */
        if(1!=fread(fragProg , lSize, 1 , fp)) {
            fclose(fp);
            free(fragProg);
            fputs("entire read fails",stderr);
            exit(1);
        }
    } else {
        vertProg = vertSource;
        fragProg = fragSource;
    }

    // stuff to actually compile the shader from https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h
    unsigned int vertex, fragment;
    // vertex
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (char const * const *)&vertProg, NULL);
    glCompileShader(vertex);
    shaderErrorCheck(vertex, "VERTEX");
    // fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (char const * const *)&fragProg, NULL);
    glCompileShader(fragment);
    shaderErrorCheck(fragment, "FRAGMENT");
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    shaderErrorCheck(ID, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (isFile) {
        free(vertProg);
        free(fragProg);
    }
    return ID;
}