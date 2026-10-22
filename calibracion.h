#ifndef CALIBRACION_H
#define CALIBRACION_H

#include <QDialog>
#include <QImage>
#include <QVector>
#include <QWidget>

class QLabel;
class QPushButton;
class QScrollArea;
class QDialogButtonBox;

enum TipoMarca { MarcaVioleta, MarcaAzul, MarcaFondo };

struct Marca {
    TipoMarca tipo;
    QPoint punto;
};

class LienzoCalibracion : public QWidget {
    Q_OBJECT

public:
    LienzoCalibracion(const QImage &imagen, QWidget *parent = nullptr);
    void setZoom(double zoom);
    double zoom() const { return factorZoom; }
    void setTipo(TipoMarca tipo) { tipoActual = tipo; }
    void deshacer();
    const QVector<Marca> &marcas() const { return listaMarcas; }

signals:
    void marcasCambiadas();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QImage imagen;
    double factorZoom = 1.0;
    TipoMarca tipoActual = MarcaVioleta;
    QVector<Marca> listaMarcas;
};

class DialogoCalibracion : public QDialog {
    Q_OBJECT

public:
    DialogoCalibracion(const QImage &imagen, QWidget *parent = nullptr);
    const QVector<Marca> &marcas() const { return lienzo->marcas(); }

private:
    void actualizarEstado();

    LienzoCalibracion *lienzo;
    QScrollArea *scroll;
    QPushButton *btnVioleta;
    QPushButton *btnAzul;
    QPushButton *btnFondo;
    QLabel *lblEstado;
    QDialogButtonBox *botones;
};

#endif // CALIBRACION_H
