#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void cargarImagen();
    void procesarImagen();
    void guardarConfiguracion();
    void cargarConfiguracion();
    void guardarImagenLayout();
    void mostrarInformacion();

private:
    QWidget *contenedorImagenes;
    // Elementos de la interfaz
    QLabel *lblImagenOriginal;
    QLabel *lblImagenDiagnostico;
    QLabel *lblImagenParametrica;
    QLabel *lblImagenResultado;
    QLabel *lblRutaArchivo;
    QLabel *lblInfoNombre;
    QLabel *lblInfoResolucion;
    QLabel *lblInfoResolucionNueva;
    QLabel *lblColoniasA;
    QLabel *lblColoniasB;
    QPushButton *btnGuardarConfig;
    QPushButton *btnGuardarImagen;
    QComboBox *comboBacterias;
    QLineEdit *txtIdPlaca;
    QPushButton *btnInfo;

    // Controles de parámetros
    QSpinBox *spinResolucion;
    QSpinBox *spinAngulo;
    QSpinBox *spinZonaMuerta;
    QSpinBox *spinRadioMin, *spinRadioMax;
    QSpinBox *spinHough1, *spinHough2;
    QSpinBox *spinReduccionRadio;
    QSpinBox *spinRMin, *spinGMin, *spinBMin;
    QSpinBox *spinRMax, *spinGMax, *spinBMax;
    QSpinBox *spinAreaMin, *spinAreaMax;

    QCheckBox *chkMostrarOriginal;
    QCheckBox *chkMostrarDiagnostico;
    QCheckBox *chkMostrarRGB;
    QCheckBox *chkMostrarResultado;
    QCheckBox *chkDividirLinea;
    QCheckBox *chkMostrarPuntos;

    // Datos de OpenCV
    cv::Mat imagenOriginal;

    int totalColoniasContadas;

    // Métodos auxiliares
    void inicializarUI();
    QSpinBox* crearSpinBox(int min, int max, int valorInicial);
    QImage matToQImage(const cv::Mat &mat);
};

#endif // MAINWINDOW_H