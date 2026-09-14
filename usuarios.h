#ifndef USUARIOS_H
#define USUARIOS_H

typedef struct GestorUsuarios GestorUsuarios;

GestorUsuarios *crearGestorUsuarios(void);

int cargarUsuariosDesdeJson(
    GestorUsuarios *gestor,
    const char *rutaArchivo);

int crearUsuario(
    GestorUsuarios *gestor,
    const char *identificacion,
    const char *nombre,
    const char *direccion);

void mostrarUsuarios(
    const GestorUsuarios *gestor);

int modificarUsuario(
    GestorUsuarios *gestor,
    const char *identificacion,
    const char *nuevoNombre,
    const char *nuevaDireccion);

int eliminarUsuario(
    GestorUsuarios *gestor,
    const char *identificacion);

int usuarioExiste(
    const GestorUsuarios *gestor,
    const char *identificacion);

int obtenerNombreUsuario(
    const GestorUsuarios *gestor,
    const char *identificacion,
    const char **nombre);

int aumentarRegistrosAsociados(
    GestorUsuarios *gestor,
    const char *identificacion);

int disminuirRegistrosAsociados(
    GestorUsuarios *gestor,
    const char *identificacion);

int guardarUsuariosJson(
    const GestorUsuarios *gestor,
    const char *rutaArchivo);

void destruirGestorUsuarios(
    GestorUsuarios *gestor);

#endif
