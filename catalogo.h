#ifndef CATALOGO_H
#define CATALOGO_H

typedef struct Catalogo Catalogo;

Catalogo *crearCatalogo(void);

int cargarCatalogoDesdeJson(
    Catalogo *catalogo,
    const char *rutaArchivo);

void incluirCatalogoPorLote(
    Catalogo *catalogo,
    const char *rutaLote,
    const char *rutaJson);

void mostrarCatalogo(
    const Catalogo *catalogo);

int obtenerNombreEjemplar(
    const Catalogo *catalogo,
    const char *identificador,
    const char **nombreLibro);

void destruirCatalogo(
    Catalogo *catalogo);

#endif