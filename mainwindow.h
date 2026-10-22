#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QImage>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
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
    void restaurarValores();

private:
    QWidget *contenedorImagenes;
    // Elementos de la interfaz
    QLabel *lblImagenOriginal;
    QLabel *lblImagenDiagnostico;
    QLabel *lblImagenParametrica;
    QLabel *lblImagenResultado;
    QLabel *lblSinImagen;
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
    QSpinBox *spinFondo, *spinUmbralOscuro;
    QSpinBox *spinUmbralMagenta, *spinSatAzul;
    QSpinBox *spinAreaMinAzul, *spinFraccionAzul;
    QSpinBox *spinAreaMin, *spinAreaMax;

    QCheckBox *chkMostrarOriginal;
    QCheckBox *chkMostrarDiagnostico;
    QCheckBox *chkMostrarRGB;
    QCheckBox *chkMostrarResultado;
    QCheckBox *chkDividirLinea;
    QCheckBox *chkMostrarPuntos;
    QCheckBox *chkRecortarPlaca;

    // Datos de OpenCV
    cv::Mat imagenOriginal;

    int totalColoniasContadas;
    bool pausarProceso = false;

    // Métodos auxiliares
    void inicializarUI();
    QSpinBox* crearSpinBox(int min, int max, int valorInicial);
    void agregarParametro(QGridLayout *layout, const QString &nombre, QSpinBox *spin);
    QImage matToQImage(const cv::Mat &mat);
    QImage componerImagenGuardado();
    void mostrarImagen(QLabel *destino, const QImage &imagen, const QString &titulo);
    void mostrarDesactivada(QLabel *destino, const QString &nombre, const QSize &tamImagen);
    void acomodarCapas();
};

#endif // MAINWINDOW_H