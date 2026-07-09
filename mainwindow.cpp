#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <cmath>
#include <QFileInfo>
#include <QComboBox>
#include <QLineEdit>
#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    totalColoniasContadas = 0; // Inicializamos la variable
    inicializarUI();
    cargarConfiguracion();
    setWindowTitle("Análisis de Placas de Petri v0.9.0 - Laboratorio de Microbiología y Biotecnología de Alimentos (MIBIAL) de la Facultad de Ciencias de la Alimentación UNER");
    setWindowIcon(QIcon("C:/Users/tomi/Documents/ContadorColonias/logo.png"));
    resize(1200, 800);
}

MainWindow::~MainWindow() {}

void MainWindow::inicializarUI() {
    QWidget *widgetCentral = new QWidget(this);
    setCentralWidget(widgetCentral);

    contenedorImagenes = new QWidget(this);
    QGridLayout *layoutImagenes = new QGridLayout(contenedorImagenes);

    // El layout principal ahora es directamente horizontal
    QHBoxLayout *layoutPrincipal = new QHBoxLayout(widgetCentral);

    lblImagenOriginal = new QLabel(this);
    lblImagenDiagnostico = new QLabel(this);
    lblImagenParametrica = new QLabel(this);
    lblImagenResultado = new QLabel(this);

    lblImagenOriginal->setAlignment(Qt::AlignCenter);
    lblImagenDiagnostico->setAlignment(Qt::AlignCenter);
    lblImagenParametrica->setAlignment(Qt::AlignCenter);
    lblImagenResultado->setAlignment(Qt::AlignCenter);

    lblImagenOriginal->setText("Sin imagen cargada");
    lblImagenDiagnostico->setText("Sin imagen cargada");
    lblImagenParametrica->setText("Sin imagen cargada");
    lblImagenResultado->setText("Sin imagen cargada");

    lblImagenOriginal->setMinimumSize(400, 300);
    lblImagenDiagnostico->setMinimumSize(400, 300);
    lblImagenParametrica->setMinimumSize(400, 300);
    lblImagenResultado->setMinimumSize(400, 300);

    layoutImagenes->addWidget(new QLabel("<b>1. Imagen Original</b>"), 0, 0, Qt::AlignCenter);
    layoutImagenes->addWidget(lblImagenOriginal, 1, 0);

    layoutImagenes->addWidget(new QLabel("<b>2. Mapa de Diagnóstico</b>"), 0, 1, Qt::AlignCenter);
    layoutImagenes->addWidget(lblImagenDiagnostico, 1, 1);

    layoutImagenes->addWidget(new QLabel("<b>3. Máscara RGB</b>"), 2, 0, Qt::AlignCenter);
    layoutImagenes->addWidget(lblImagenParametrica, 3, 0);

    layoutImagenes->addWidget(new QLabel("<b>4. Resultado Final</b>"), 2, 1, Qt::AlignCenter);
    layoutImagenes->addWidget(lblImagenResultado, 3, 1);

    // Le decimos al contenedor que tiene permiso para expandirse
    contenedorImagenes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // AGREGAMOS EL WIDGET (contenedorImagenes), NO EL LAYOUT (layoutImagenes)
    layoutPrincipal->addWidget(contenedorImagenes, 3);

    // --- LADO DERECHO: Columna de Controles ---
    QVBoxLayout *layoutDerecho = new QVBoxLayout();

    // 1. Botón de carga e Información
    QPushButton *btnCargar = new QPushButton("Cargar Imagen", this);
    QPushButton *btnGuardarImagen = new QPushButton("Guardar imágenes", this);
    QPushButton *btnGuardarConfig = new QPushButton("Guardar config.", this);
    btnInfo = new QPushButton("Acerca de", this);

    lblInfoNombre = new QLabel("Nombre: -", this);
    lblInfoResolucion = new QLabel("Resolución original: -", this);
    lblInfoResolucionNueva = new QLabel("Resolución actual: -", this);
    lblColoniasA = new QLabel("Cantidad de colonias (Rojo): -", this);
    lblColoniasB = new QLabel("Cantidad de colonias (Azul): -", this);
    lblInfoNombre->setWordWrap(true);

    comboBacterias = new QComboBox(this);

    comboBacterias->addItems({"Escherichia", "Staphylococcus", "Bacillus"});
    comboBacterias->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    txtIdPlaca = new QLineEdit(this);
    txtIdPlaca->setPlaceholderText("Ej: 24534"); // Texto de ayuda que aparece en gris

    QHBoxLayout *layoutBotones1 = new QHBoxLayout();
    layoutBotones1->addWidget(btnCargar);
    layoutBotones1->addWidget(btnGuardarImagen);
    layoutDerecho->addLayout(layoutBotones1);

    // Permitir que nombres largos bajen de línea
    lblInfoNombre->setWordWrap(true);

    QGridLayout *layoutDatosPlaca = new QGridLayout();
    layoutDatosPlaca->addWidget(new QLabel("<b>Tipo de Bacteria:</b>", this), 0, 0); // Fila 0, Columna 0
    layoutDatosPlaca->addWidget(new QLabel("<b>ID de la Placa:</b>", this), 0, 1);   // Fila 0, Columna 1
    layoutDatosPlaca->addWidget(comboBacterias, 1, 0);                               // Fila 1, Columna 0
    layoutDatosPlaca->addWidget(txtIdPlaca, 1, 1);                                   // Fila 1, Columna 1

    // Sumamos la cuadrícula entera a la columna derecha
    layoutDerecho->addLayout(layoutDatosPlaca);

    QHBoxLayout *layoutBotones2 = new QHBoxLayout();
    layoutBotones2->addWidget(btnGuardarConfig);
    layoutBotones2->addWidget(btnInfo);
    layoutDerecho->addLayout(layoutBotones2);

    layoutDerecho->addSpacing(15);
    layoutDerecho->addWidget(new QLabel("<b>Información:</b>", this));
    layoutDerecho->addWidget(lblInfoNombre);
    layoutDerecho->addWidget(lblInfoResolucion);
    layoutDerecho->addWidget(lblInfoResolucionNueva);
    layoutDerecho->addWidget(lblColoniasA);
    layoutDerecho->addWidget(lblColoniasB);
    layoutDerecho->addSpacing(15);

    QGridLayout *layoutChecks = new QGridLayout();

    chkMostrarOriginal = new QCheckBox("Mostrar Original", this);
    chkMostrarDiagnostico = new QCheckBox("Mostrar Diagnóstico", this);
    chkMostrarRGB = new QCheckBox("Mostrar RGB", this);
    chkMostrarResultado = new QCheckBox("Mostrar Resultado", this);
    chkDividirLinea = new QCheckBox("Dividir con Línea", this);
    chkMostrarPuntos = new QCheckBox("Mostrar Puntos", this);

    // Activarlos todos por defecto al arrancar
    chkMostrarOriginal->setChecked(true);
    chkMostrarDiagnostico->setChecked(true);
    chkMostrarRGB->setChecked(true);
    chkMostrarResultado->setChecked(true);
    chkDividirLinea->setChecked(true);
    chkMostrarPuntos->setChecked(true);

    // Acomodarlos en 2 columnas (Fila, Columna)
    layoutChecks->addWidget(chkMostrarOriginal, 0, 0);
    layoutChecks->addWidget(chkMostrarDiagnostico, 0, 1);
    layoutChecks->addWidget(chkMostrarRGB, 1, 0);
    layoutChecks->addWidget(chkMostrarResultado, 1, 1);
    layoutChecks->addWidget(chkDividirLinea, 2, 0);
    layoutChecks->addWidget(chkMostrarPuntos, 2, 1);

    layoutDerecho->addLayout(layoutChecks);
    layoutDerecho->addSpacing(15);

    // 2. Formulario de Parámetros (Ahora en QGridLayout de 2 columnas)
    QGridLayout *layoutParametros = new QGridLayout();

    spinResolucion = crearSpinBox(100, 2000, 800);
    spinAngulo = crearSpinBox(0, 360, 65);
    spinZonaMuerta = crearSpinBox(0, 100, 15);
    spinRadioMin = crearSpinBox(10, 1000, 150);
    spinRadioMax = crearSpinBox(10, 1000, 400);
    spinHough1 = crearSpinBox(1, 255, 50);
    spinHough2 = crearSpinBox(1, 100, 30);
    spinReduccionRadio = crearSpinBox(0, 100, 12);
    spinRMin = crearSpinBox(0, 255, 160);
    spinGMin = crearSpinBox(0, 255, 160);
    spinBMin = crearSpinBox(0, 255, 140);
    spinRMax = crearSpinBox(0, 255, 255);
    spinGMax = crearSpinBox(0, 255, 255);
    spinBMax = crearSpinBox(0, 255, 255);
    spinAreaMin = crearSpinBox(0, 500, 1);
    spinAreaMax = crearSpinBox(1, 5000, 200);

    layoutDerecho->addWidget(new QLabel("<b>Parametros:</b>", this));
    // Fila 0
    layoutParametros->addWidget(new QLabel("Ancho Proc:"), 0, 0);
    layoutParametros->addWidget(spinResolucion, 0, 1);
    layoutParametros->addWidget(new QLabel("Ángulo Div:"), 0, 2);
    layoutParametros->addWidget(spinAngulo, 0, 3);

    // Fila 1
    layoutParametros->addWidget(new QLabel("Zona Muerta:"), 1, 0);
    layoutParametros->addWidget(spinZonaMuerta, 1, 1);
    layoutParametros->addWidget(new QLabel("Radio Mín:"), 1, 2);
    layoutParametros->addWidget(spinRadioMin, 1, 3);

    // Fila 2
    layoutParametros->addWidget(new QLabel("Radio Máx:"), 2, 0);
    layoutParametros->addWidget(spinRadioMax, 2, 1);
    layoutParametros->addWidget(new QLabel("Hough 1:"), 2, 2);
    layoutParametros->addWidget(spinHough1, 2, 3);

    // Fila 3
    layoutParametros->addWidget(new QLabel("Hough 2:"), 3, 0);
    layoutParametros->addWidget(spinHough2, 3, 1);
    layoutParametros->addWidget(new QLabel("Red. Borde:"), 3, 2);
    layoutParametros->addWidget(spinReduccionRadio, 3, 3);

    // Fila 4
    layoutParametros->addWidget(new QLabel("Rojo Mín:"), 4, 0);
    layoutParametros->addWidget(spinRMin, 4, 1);
    layoutParametros->addWidget(new QLabel("Rojo Máx:"), 4, 2);
    layoutParametros->addWidget(spinRMax, 4, 3);

    // Fila 5
    layoutParametros->addWidget(new QLabel("Verde Mín:"), 5, 0);
    layoutParametros->addWidget(spinGMin, 5, 1);
    layoutParametros->addWidget(new QLabel("Verde Máx:"), 5, 2);
    layoutParametros->addWidget(spinGMax, 5, 3);

    // Fila 6
    layoutParametros->addWidget(new QLabel("Azul Mín:"), 6, 0);
    layoutParametros->addWidget(spinBMin, 6, 1);
    layoutParametros->addWidget(new QLabel("Azul Máx:"), 6, 2);
    layoutParametros->addWidget(spinBMax, 6, 3);

    // Fila 7
    layoutParametros->addWidget(new QLabel("Área Mín:"), 7, 0);
    layoutParametros->addWidget(spinAreaMin, 7, 1);
    layoutParametros->addWidget(new QLabel("Área Máx:"), 7, 2);
    layoutParametros->addWidget(spinAreaMax, 7, 3);

    // Finalmente, lo sumamos al layoutDerecho (asegurate de cambiar addLayout a este)
    layoutDerecho->addLayout(layoutParametros);

    layoutDerecho->addStretch(); // Empuja todos los controles hacia arriba para que no queden flotando

    layoutPrincipal->addLayout(layoutDerecho, 1); // Otorga 1/4 del espacio a los controles

    connect(btnCargar, &QPushButton::clicked, this, &MainWindow::cargarImagen);
    connect(btnGuardarConfig, &QPushButton::clicked, this, &MainWindow::guardarConfiguracion);
    connect(btnGuardarImagen, &QPushButton::clicked, this, &MainWindow::guardarImagenLayout);
    connect(btnInfo, &QPushButton::clicked, this, &MainWindow::mostrarInformacion);
    connect(chkMostrarOriginal, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
    connect(chkMostrarDiagnostico, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
    connect(chkMostrarRGB, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
    connect(chkMostrarResultado, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
    connect(chkDividirLinea, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
    connect(chkMostrarPuntos, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
}

QSpinBox* MainWindow::crearSpinBox(int min, int max, int valorInicial) {
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(min, max);
    spin->setValue(valorInicial);

    // NUEVO: Fija el ancho del cuadro para que no ocupe toda la pantalla
    spin->setMaximumWidth(60);

    // Conectar el cambio de valor al reprocesamiento automático
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::procesarImagen);
    return spin;
}

void MainWindow::cargarImagen() {
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Imagen", "", "Images (*.png *.xpm *.jpg *.jpeg)");

    if (!fileName.isEmpty()) {
        imagenOriginal = cv::imread(fileName.toStdString());

        if (imagenOriginal.empty()) {
            qDebug() << "[ERROR] No se pudo cargar la imagen en la ruta:" << fileName;
            return;
        }

        // Extraer solo el nombre del archivo de la ruta completa
        QFileInfo fileInfo(fileName);
        QString nombreArchivo = fileInfo.fileName();

        // Extraer dimensiones de la matriz de OpenCV
        int ancho = imagenOriginal.cols;
        int alto = imagenOriginal.rows;

        // Actualizar la interfaz
        lblInfoNombre->setText("Nombre: " + nombreArchivo);
        lblInfoResolucion->setText("Resolución original: " + QString::number(ancho) + " x " + QString::number(alto) + " px");

        qDebug() << "[LOG] Imagen cargada exitosamente. Nombre:" << nombreArchivo << "- Dimensiones:" << ancho << "x" << alto;

        procesarImagen();
    }
}

void MainWindow::procesarImagen() {
    if (imagenOriginal.empty()) return;

    qDebug() << "----------------------------------------";
    qDebug() << "[LOG] Iniciando procesamiento paramétrico (Con ROI)...";

    cv::Mat imgRedimensionada;
    int anchoObj = spinResolucion->value();
    int altoNuevo;

    if (imagenOriginal.cols > anchoObj) {
        double proporcion = (double)anchoObj / imagenOriginal.cols;
        altoNuevo = imagenOriginal.rows * proporcion;
        cv::resize(imagenOriginal, imgRedimensionada, cv::Size(anchoObj, altoNuevo), 0, 0, cv::INTER_AREA);
    } else {
        imgRedimensionada = imagenOriginal.clone();
        altoNuevo = imgRedimensionada.rows;
    }

    lblInfoResolucionNueva->setText("Resolución actual: " + QString::number(anchoObj) + " x " + QString::number(altoNuevo) + " px");

    cv::Mat imgRGB, imgResultado, mascaraDiagnostico;
    cv::cvtColor(imgRedimensionada, imgRGB, cv::COLOR_BGR2RGB);

    if (chkMostrarResultado->isChecked()) {
        imgResultado = imgRGB.clone();
    }
    if (chkMostrarDiagnostico->isChecked()) {
        mascaraDiagnostico = cv::Mat::zeros(imgRedimensionada.size(), CV_8UC3);
    }

    // 1. Detección de la placa
    cv::Mat gray, gray_blur;
    cv::cvtColor(imgRedimensionada, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray_blur, 9);

    std::vector<cv::Vec3f> circulos;
    cv::HoughCircles(gray_blur, circulos, cv::HOUGH_GRADIENT, 1.2, 100,
                     spinHough1->value(), spinHough2->value(),
                     spinRadioMin->value(), spinRadioMax->value());

    cv::Mat mascara_placa = cv::Mat::zeros(gray.size(), CV_8UC1);

    int x_circulo = gray.cols / 2;
    int y_circulo = gray.rows / 2;
    int r_circulo = std::min(gray.cols, gray.rows) / 2;

    if (!circulos.empty()) {
        x_circulo = cvRound(circulos[0][0]);
        y_circulo = cvRound(circulos[0][1]);
        r_circulo = cvRound(circulos[0][2]);

        if (chkMostrarResultado->isChecked()) {
            cv::circle(imgResultado, cv::Point(x_circulo, y_circulo), r_circulo, cv::Scalar(0, 255, 0), 3);
        }

        int radio_util = std::max(1, r_circulo - spinReduccionRadio->value());
        cv::circle(mascara_placa, cv::Point(x_circulo, y_circulo), radio_util, cv::Scalar(255), cv::FILLED);
        qDebug() << "[LOG] Círculo detectado en X:" << x_circulo << "Y:" << y_circulo << "Radio:" << r_circulo;
    } else {
        qDebug() << "[ADVERTENCIA] No se detectó círculo. Se usa toda la imagen.";
        mascara_placa.setTo(cv::Scalar(255));
    }

    // Cálculo matemático de la recta
    double angulo_rad = spinAngulo->value() * CV_PI / 180.0;
    int x1 = x_circulo + r_circulo * std::cos(angulo_rad);
    int y1 = y_circulo - r_circulo * std::sin(angulo_rad);
    int x2 = x_circulo - r_circulo * std::cos(angulo_rad);
    int y2 = y_circulo + r_circulo * std::sin(angulo_rad);
    double norma_recta = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));

    if (chkDividirLinea->isChecked()) {
        if (chkMostrarResultado->isChecked()) {
            cv::line(imgResultado, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 255, 0), 3);
        }
        if (chkMostrarDiagnostico->isChecked()) {
            cv::line(mascaraDiagnostico, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(50, 50, 50), 1);
        }
    }

    cv::Mat imgAislada;
    cv::bitwise_and(imgRGB, imgRGB, imgAislada, mascara_placa);

    // --- NUEVO: OPTIMIZACIÓN CON ROI (Región de Interés) ---
    // 1. Calculamos el cuadrado exacto que encierra al círculo sin pasarnos de los bordes
    int roi_x = std::max(0, x_circulo - r_circulo);
    int roi_y = std::max(0, y_circulo - r_circulo);
    int roi_w = std::min(imgRedimensionada.cols - roi_x, r_circulo * 2);
    int roi_h = std::min(imgRedimensionada.rows - roi_y, r_circulo * 2);
    cv::Rect roi(roi_x, roi_y, roi_w, roi_h);

    // 2. Preparamos una máscara negra del tamaño total
    cv::Mat mascara_colonias = cv::Mat::zeros(imgRedimensionada.size(), CV_8UC1);

    // 3. Segmentamos SOLO el cuadradito (Ahorra procesar miles de píxeles negros inútiles)
    if (roi.width > 0 && roi.height > 0) {
        cv::Mat imgAislada_ROI = imgAislada(roi);
        cv::Mat mascara_colonias_ROI;
        cv::Scalar colorMin(spinRMin->value(), spinGMin->value(), spinBMin->value());
        cv::Scalar colorMax(spinRMax->value(), spinGMax->value(), spinBMax->value());
        cv::inRange(imgAislada_ROI, colorMin, colorMax, mascara_colonias_ROI);

        // 4. Pegamos el cuadradito ya procesado en nuestra máscara grande
        mascara_colonias_ROI.copyTo(mascara_colonias(roi));
    }

    // 3. Extracción de momentos y evaluación espacial (Tú código original)
    std::vector<std::vector<cv::Point>> contornos;
    cv::findContours(mascara_colonias, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (chkDividirLinea->isChecked() && chkMostrarDiagnostico->isChecked()) {
        cv::line(mascaraDiagnostico, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(50, 50, 50), 1);
    }

    int ladoA = 0, ladoB = 0, ignoradas = 0;
    int zonaMuertaPx = spinZonaMuerta->value();

    for (size_t i = 0; i < contornos.size(); i++) {
        double area = cv::contourArea(contornos[i]);

        if (chkMostrarDiagnostico->isChecked()) {
            if (area <= spinAreaMin->value()) {
                cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(150, 0, 150), cv::FILLED);
            } else if (area >= spinAreaMax->value()) {
                cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(255, 255, 0), cv::FILLED);
            }
        }

        if (area > spinAreaMin->value() && area < spinAreaMax->value()) {
            cv::Moments M = cv::moments(contornos[i]);
            if (M.m00 != 0) {
                int cX = static_cast<int>(M.m10 / M.m00);
                int cY = static_cast<int>(M.m01 / M.m00);

                if (chkDividirLinea->isChecked()) {
                    double numerador = std::abs((x2 - x1) * (y1 - cY) - (x1 - cX) * (y2 - y1));
                    double distancia_ortogonal = numerador / norma_recta;

                    if (distancia_ortogonal <= zonaMuertaPx) {
                        if (chkMostrarDiagnostico->isChecked()) {
                            cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(100, 100, 100), cv::FILLED);
                        }
                        ignoradas++;
                        continue;
                    }

                    if (chkMostrarDiagnostico->isChecked()) {
                        cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(0, 255, 0), cv::FILLED);
                    }

                    double val_lado = (x1 - x2) * (cY - y2) - (y1 - y2) * (cX - x2);
                    if (val_lado > 0) {
                        ladoA++;
                        if (chkMostrarResultado->isChecked() && chkMostrarPuntos->isChecked())
                            cv::circle(imgResultado, cv::Point(cX, cY), 3, cv::Scalar(255, 0, 0), cv::FILLED);
                    } else {
                        ladoB++;
                        if (chkMostrarResultado->isChecked() && chkMostrarPuntos->isChecked())
                            cv::circle(imgResultado, cv::Point(cX, cY), 3, cv::Scalar(0, 0, 255), cv::FILLED);
                    }
                } else {
                    if (chkMostrarDiagnostico->isChecked()) {
                        cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(0, 255, 0), cv::FILLED);
                    }
                    ladoB++;
                    if (chkMostrarResultado->isChecked() && chkMostrarPuntos->isChecked()) {
                        cv::circle(imgResultado, cv::Point(cX, cY), 3, cv::Scalar(0, 0, 255), cv::FILLED);
                    }
                }
            }
        }
    }

    totalColoniasContadas = ladoA + ladoB;

    qDebug() << "[RESULTADOS] Lado A:" << ladoA << "| Lado B:" << ladoB << "| Ignoradas:" << ignoradas;

    lblColoniasA->setText("Cantidad de colonias (Rojo): " + QString::number(ladoA));
    lblColoniasB->setText("Cantidad de colonias (Azul): " + QString::number(ladoB));

    // --- RENDERIZADO EN UI ---
    if (chkMostrarOriginal->isChecked()) {
        QImage qOriginal = matToQImage(imgRGB);
        lblImagenOriginal->setPixmap(QPixmap::fromImage(qOriginal).scaled(lblImagenOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblImagenOriginal->clear();
        lblImagenOriginal->setText("Desactivada para ahorrar memoria");
    }

    if (chkMostrarDiagnostico->isChecked()) {
        QImage qDiagnostico = matToQImage(mascaraDiagnostico);
        lblImagenDiagnostico->setPixmap(QPixmap::fromImage(qDiagnostico).scaled(lblImagenDiagnostico->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblImagenDiagnostico->clear();
        lblImagenDiagnostico->setText("Desactivada para ahorrar memoria");
    }

    if (chkMostrarRGB->isChecked()) {
        QImage qParametrica = matToQImage(mascara_colonias);
        lblImagenParametrica->setPixmap(QPixmap::fromImage(qParametrica).scaled(lblImagenParametrica->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblImagenParametrica->clear();
        lblImagenParametrica->setText("Desactivada para ahorrar memoria");
    }

    if (chkMostrarResultado->isChecked()) {
        QImage qResultado = matToQImage(imgResultado);
        lblImagenResultado->setPixmap(QPixmap::fromImage(qResultado).scaled(lblImagenResultado->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblImagenResultado->clear();
        lblImagenResultado->setText("Desactivada para ahorrar memoria");
    }
}

QImage MainWindow::matToQImage(const cv::Mat &mat) {
    if (mat.type() == CV_8UC1) {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return image.copy();
    } else if (mat.type() == CV_8UC3) {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.copy();
    }
    return QImage();
}

// --- NUEVAS FUNCIONES PARA GUARDAR CONFIGURACIONES E IMÁGENES ---

void MainWindow::guardarConfiguracion() {
    // QSettings guarda estos datos en el sistema operativo
    QSettings settings("MiLaboratorio", "ContadorColonias");

    settings.setValue("resolucion", spinResolucion->value());
    settings.setValue("angulo", spinAngulo->value());
    settings.setValue("zonaMuerta", spinZonaMuerta->value());
    settings.setValue("radioMin", spinRadioMin->value());
    settings.setValue("radioMax", spinRadioMax->value());
    settings.setValue("hough1", spinHough1->value());
    settings.setValue("hough2", spinHough2->value());
    settings.setValue("redRadio", spinReduccionRadio->value());

    settings.setValue("rMin", spinRMin->value());
    settings.setValue("gMin", spinGMin->value());
    settings.setValue("bMin", spinBMin->value());
    settings.setValue("rMax", spinRMax->value());
    settings.setValue("gMax", spinGMax->value());
    settings.setValue("bMax", spinBMax->value());

    settings.setValue("areaMin", spinAreaMin->value());
    settings.setValue("areaMax", spinAreaMax->value());

    QMessageBox::information(this, "Éxito", "Configuración guardada por defecto.");
}

void MainWindow::cargarConfiguracion() {
    QSettings settings("MiLaboratorio", "ContadorColonias");

    // El segundo parámetro de value() es el valor que toma si no encuentra nada guardado
    spinResolucion->setValue(settings.value("resolucion", 800).toInt());
    spinAngulo->setValue(settings.value("angulo", 65).toInt());
    spinZonaMuerta->setValue(settings.value("zonaMuerta", 15).toInt());
    spinRadioMin->setValue(settings.value("radioMin", 150).toInt());
    spinRadioMax->setValue(settings.value("radioMax", 400).toInt());
    spinHough1->setValue(settings.value("hough1", 50).toInt());
    spinHough2->setValue(settings.value("hough2", 30).toInt());
    spinReduccionRadio->setValue(settings.value("redRadio", 12).toInt());

    spinRMin->setValue(settings.value("rMin", 160).toInt());
    spinGMin->setValue(settings.value("gMin", 160).toInt());
    spinBMin->setValue(settings.value("bMin", 140).toInt());
    spinRMax->setValue(settings.value("rMax", 255).toInt());
    spinGMax->setValue(settings.value("gMax", 255).toInt());
    spinBMax->setValue(settings.value("bMax", 255).toInt());

    spinAreaMin->setValue(settings.value("areaMin", 1).toInt());
    spinAreaMax->setValue(settings.value("areaMax", 200).toInt());
}

void MainWindow::guardarImagenLayout() {
    if (imagenOriginal.empty()) {
        QMessageBox::warning(this, "Aviso", "No hay ninguna imagen procesada para guardar.");
        return;
    }

    // 1. Obtener la fecha en formato yyMMdd (ej. 260705 para el 5 de julio de 2026)
    QString fechaActual = QDate::currentDate().toString("yyMMdd");

    // 2. Obtener el nombre de la bacteria y pasarlo a minúsculas
    QString bacteria = comboBacterias->currentText().toLower();

    // 3. Obtener el ID de la placa (si está vacío, le ponemos un "sin_id")
    QString idPlaca = txtIdPlaca->text().trimmed();
    if (idPlaca.isEmpty()) idPlaca = "sin_id";

    // 4. Armar el nombre del archivo: bacteria + total_colonias + _ + id + _ + fecha
    QString nombreSugerido = QString("%1%2_%3_%4.png")
                                 .arg(bacteria)
                                 .arg(totalColoniasContadas)
                                 .arg(idPlaca)
                                 .arg(fechaActual);

    // 5. Abrir la ventana para elegir dónde guardarlo
    QString rutaGuardado = QFileDialog::getSaveFileName(this, "Guardar Resultados", nombreSugerido, "Images (*.png)");

    if (!rutaGuardado.isEmpty()) {
        // Sacar la "captura" al widget que contiene las 4 imágenes
        QPixmap captura = contenedorImagenes->grab();
        if (captura.save(rutaGuardado)) {
            qDebug() << "[LOG] Layout guardado en:" << rutaGuardado;
            QMessageBox::information(this, "Éxito", "Imagen guardada correctamente.");
        } else {
            QMessageBox::critical(this, "Error", "No se pudo guardar la imagen.");
        }
    }
}
void MainWindow::mostrarInformacion() {
    QString textoInfo =
        "<h2>Análisis de Placas de Petri v0.9.0</h2>"
        "<p>Software paramétrico de visión artificial para el conteo automático de unidades formadoras de colonias.</p>"
        "<hr>"
        "<p><b>Desarrollado para:</b><br>"
        "Laboratorio de Microbiología y Biotecnología de Alimentos (MIBIAL)<br>"
        "Facultad de Ciencias de la Alimentación - UNER</p>"
        "<p><b>Motor de Procesamiento:</b> OpenCV y Qt6 C++</p>"
        "<p><b>Uso de Parámetros:</b><br>"
                        "- <i>Ancho Proc:</i> Resolución interna de cálculo. <b>Recomendación:</b> No cambiar. Alterarlo desconfigura la escala de radios y áreas.<br>"
                        "- <i>Ángulo Div:</i> Inclinación de la recta separadora. <b>Recomendación:</b> Ajustar en cada foto para alinear con la división física de la placa.<br>"
                        "- <i>Zona Muerta:</i> Margen ignorado en la línea divisoria. <b>Recomendación:</b> Ajustar solo si hay ruido o bacterias pisando la raya.<br>"
                        "- <i>Radio Mín / Máx:</i> Tamaño del círculo de la placa en píxeles. <b>Recomendación:</b> Cambiar solo si la cámara se acercó o alejó mucho.<br>"
                        "- <i>Hough 1 y 2:</i> Sensibilidad matemática del detector de la placa. <b>Recomendación:</b> No tocar a menos que el círculo verde falle.<br>"
                        "- <i>Red. Borde:</i> Margen interno para recortar la imagen. <b>Recomendación:</b> Aumentar si el brillo del borde del plástico cuenta como falsas bacterias.<br>"
                        "- <i>Filtros RGB (Mín/Máx):</i> Segmentación del color de las colonias. <b>Recomendación:</b> Calibrar siempre según la iluminación para que solo se vean las bacterias en la vista RGB.<br>"
                        "- <i>Área Mín / Máx:</i> Filtro de tamaño geométrico. <b>Recomendación:</b> Subir el Mínimo para ignorar polvo o ruido; bajar el Máximo para ignorar grietas o reflejos grandes.</p>";

    // El QMessageBox::about es especial para estas ventanas de información institucional
    QMessageBox::about(this, "Acerca del Sistema", textoInfo);
}