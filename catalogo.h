#ifndef CATALOGO_H
#define CATALOGO_H

typedef struct Catalogo Catalogo;

struct GestorPrestamos;

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

    void buscarCatalogoSimple(
    const Catalogo *catalogo,
    const struct GestorPrestamos *gestorPrestamos,
    const char *texto);
    
int obtenerNombreEjemplar(
    const Catalogo *catalogo,
    const char *identificador,
    const char **nombreLibro);

void buscarCatalogoAvanzado(
    const Catalogo *catalogo,
    const char *nombre, int tecnicaNombre,
    const char *autor, int tecnicaAutor,
    const char *genero, int tecnicaGenero,
    const char *resumen, int tecnicaResumen,
    int operador);

void destruirCatalogo(
    Catalogo *catalogo);

#endif
