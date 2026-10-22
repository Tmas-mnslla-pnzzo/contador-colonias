#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <cmath>
#include <climits>
#include <QFileInfo>
#include <QComboBox>
#include <QLineEdit>
#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QIcon>
#include <QPainter>
#include <QTabWidget>
#include <QSlider>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    totalColoniasContadas = 0; // Inicializamos la variable
    inicializarUI();
    cargarConfiguracion();
    setWindowTitle("Análisis de Placas de Petri v" APP_VERSION " - Laboratorio de Microbiología y Biotecnología de Alimentos (MIBIAL) de la Facultad de Ciencias de la Alimentación UNER");
    setWindowIcon(QIcon(":/icono.svg"));
    QTimer::singleShot(1000, this, &MainWindow::buscarActualizacion);
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

    lblImagenOriginal->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    lblImagenDiagnostico->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    lblImagenParametrica->setAlignment(Qt::AlignRight | Qt::AlignTop);
    lblImagenResultado->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    lblImagenOriginal->setProperty("alineacion", int(Qt::AlignRight | Qt::AlignBottom));
    lblImagenDiagnostico->setProperty("alineacion", int(Qt::AlignLeft | Qt::AlignBottom));
    lblImagenParametrica->setProperty("alineacion", int(Qt::AlignRight | Qt::AlignTop));
    lblImagenResultado->setProperty("alineacion", int(Qt::AlignLeft | Qt::AlignTop));

    lblSinImagen = new QLabel("Sin imagen cargada", this);
    lblSinImagen->setAlignment(Qt::AlignCenter);
    lblSinImagen->setMinimumSize(806, 606);

    lblImagenOriginal->setMinimumSize(400, 300);
    lblImagenDiagnostico->setMinimumSize(400, 300);
    lblImagenParametrica->setMinimumSize(400, 300);
    lblImagenResultado->setMinimumSize(400, 300);

    layoutImagenes->addWidget(lblImagenOriginal, 0, 0);
    layoutImagenes->addWidget(lblImagenDiagnostico, 0, 1);
    layoutImagenes->addWidget(lblImagenParametrica, 1, 0);
    layoutImagenes->addWidget(lblImagenResultado, 1, 1);
    layoutImagenes->addWidget(lblSinImagen, 0, 0, 2, 2);
    layoutImagenes->setRowStretch(0, 1);
    layoutImagenes->setRowStretch(1, 1);
    layoutImagenes->setColumnStretch(0, 1);
    layoutImagenes->setColumnStretch(1, 1);

    for (QLabel *lbl : {lblImagenOriginal, lblImagenDiagnostico, lblImagenParametrica, lblImagenResultado}) {
        lbl->hide();
    }

    layoutImagenes->setSpacing(6);
    layoutImagenes->setContentsMargins(0, 0, 0, 0);

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
    lblColoniasA = new QLabel("Colonias violetas: -", this);
    lblColoniasB = new QLabel("Colonias azules: -", this);
    lblInfoNombre->setWordWrap(true);

    comboBacterias = new QComboBox(this);

    comboBacterias->addItems({"Escherichia coli", "Coliformes totales", "Aerobios mesófilos totales",
                              "Pseudomonas aeruginosa", "Bacillus", "Staphylococcus"});
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

    QPushButton *btnAutocalibracion = new QPushButton("Autocalibración", this);
    layoutDerecho->addWidget(btnAutocalibracion);
    connect(btnAutocalibracion, &QPushButton::clicked, this, &MainWindow::abrirAutocalibracion);

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
    chkMostrarRGB = new QCheckBox("Mostrar Máscara", this);
    chkMostrarResultado = new QCheckBox("Mostrar Resultado", this);
    chkDividirLinea = new QCheckBox("Dividir con Línea", this);
    chkMostrarPuntos = new QCheckBox("Mostrar Puntos", this);
    chkRecortarPlaca = new QCheckBox("Recortar a la Placa", this);

    // Activarlos todos por defecto al arrancar
    chkMostrarOriginal->setChecked(true);
    chkMostrarDiagnostico->setChecked(true);
    chkMostrarRGB->setChecked(true);
    chkMostrarResultado->setChecked(true);
    chkDividirLinea->setChecked(true);
    chkMostrarPuntos->setChecked(true);
    chkRecortarPlaca->setChecked(true);

    // Acomodarlos en 2 columnas (Fila, Columna)
    layoutChecks->addWidget(chkMostrarOriginal, 0, 0);
    layoutChecks->addWidget(chkMostrarDiagnostico, 0, 1);
    layoutChecks->addWidget(chkMostrarRGB, 1, 0);
    layoutChecks->addWidget(chkMostrarResultado, 1, 1);
    layoutChecks->addWidget(chkDividirLinea, 2, 0);
    layoutChecks->addWidget(chkMostrarPuntos, 2, 1);
    layoutChecks->addWidget(chkRecortarPlaca, 3, 0);

    layoutDerecho->addLayout(layoutChecks);
    layoutDerecho->addSpacing(15);

    // 2. Parámetros agrupados en pestañas, cada uno con slider y número
    spinResolucion = crearSpinBox(100, 2000, 800);
    spinAngulo = crearSpinBox(0, 360, 65);
    spinZonaMuerta = crearSpinBox(0, 100, 15);
    spinRadioMin = crearSpinBox(10, 1000, 150);
    spinRadioMax = crearSpinBox(10, 1000, 400);
    spinHough1 = crearSpinBox(1, 255, 50);
    spinHough2 = crearSpinBox(1, 100, 30);
    spinReduccionRadio = crearSpinBox(0, 100, 12);
    spinFondo = crearSpinBox(3, 151, 31);
    spinUmbralOscuro = crearSpinBox(1, 100, 25);
    spinUmbralMagenta = crearSpinBox(-20, 30, 0);
    spinSatAzul = crearSpinBox(0, 255, 45);
    spinAreaMinAzul = crearSpinBox(0, 1000, 15);
    spinFraccionAzul = crearSpinBox(0, 100, 30);
    spinAreaMin = crearSpinBox(0, 500, 2);
    spinAreaMax = crearSpinBox(1, 5000, 800);

    layoutDerecho->addWidget(new QLabel("<b>Parámetros:</b>", this));

    QTabWidget *pestanas = new QTabWidget(this);

    QWidget *pagDivision = new QWidget(this);
    QGridLayout *layDivision = new QGridLayout(pagDivision);
    agregarParametro(layDivision, "Ángulo división", spinAngulo);
    agregarParametro(layDivision, "Zona muerta", spinZonaMuerta);
    layDivision->setRowStretch(layDivision->rowCount(), 1);
    pestanas->addTab(pagDivision, "División");

    QWidget *pagDeteccion = new QWidget(this);
    QGridLayout *layDeteccion = new QGridLayout(pagDeteccion);
    agregarParametro(layDeteccion, "Umbral oscuro", spinUmbralOscuro);
    agregarParametro(layDeteccion, "Umbral violeta", spinUmbralMagenta);
    agregarParametro(layDeteccion, "Área mínima", spinAreaMin);
    agregarParametro(layDeteccion, "Área máxima", spinAreaMax);
    agregarParametro(layDeteccion, "Saturación azul", spinSatAzul);
    agregarParametro(layDeteccion, "% azul", spinFraccionAzul);
    agregarParametro(layDeteccion, "Área mín. azul", spinAreaMinAzul);
    layDeteccion->setRowStretch(layDeteccion->rowCount(), 1);
    pestanas->addTab(pagDeteccion, "Detección");

    QWidget *pagPlaca = new QWidget(this);
    QGridLayout *layPlaca = new QGridLayout(pagPlaca);
    agregarParametro(layPlaca, "Hough 1", spinHough1);
    agregarParametro(layPlaca, "Hough 2", spinHough2);
    agregarParametro(layPlaca, "Radio mínimo", spinRadioMin);
    agregarParametro(layPlaca, "Radio máximo", spinRadioMax);
    agregarParametro(layPlaca, "Reducción borde", spinReduccionRadio);
    agregarParametro(layPlaca, "Ancho proceso", spinResolucion);
    agregarParametro(layPlaca, "Fondo (px)", spinFondo);
    layPlaca->setRowStretch(layPlaca->rowCount(), 1);
    pestanas->addTab(pagPlaca, "Placa (avanzado)");

    layoutDerecho->addWidget(pestanas);

    QPushButton *btnRestaurar = new QPushButton("Restaurar valores", this);
    layoutDerecho->addWidget(btnRestaurar, 0, Qt::AlignRight);
    connect(btnRestaurar, &QPushButton::clicked, this, &MainWindow::restaurarValores);

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
    connect(chkRecortarPlaca, &QCheckBox::toggled, this, &MainWindow::procesarImagen);
}

QSpinBox* MainWindow::crearSpinBox(int min, int max, int valorInicial) {
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(min, max);
    spin->setValue(valorInicial);
    spin->setProperty("defecto", valorInicial);

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
    if (imagenOriginal.empty() || pausarProceso) return;

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

    lblInfoResolucionNueva->setText("Resolución actual: " + QString::number(imgRedimensionada.cols) + " x " + QString::number(altoNuevo) + " px");

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

    int roi_x = std::max(0, x_circulo - r_circulo);
    int roi_y = std::max(0, y_circulo - r_circulo);
    int roi_w = std::min(imgRedimensionada.cols - roi_x, r_circulo * 2);
    int roi_h = std::min(imgRedimensionada.rows - roi_y, r_circulo * 2);
    cv::Rect roi(roi_x, roi_y, roi_w, roi_h);

    cv::Mat mascara_colonias = cv::Mat::zeros(imgRedimensionada.size(), CV_8UC1);
    cv::Mat deltaMagenta = cv::Mat::zeros(imgRedimensionada.size(), CV_32FC1);
    cv::Mat mascaraAzul = cv::Mat::zeros(imgRedimensionada.size(), CV_8UC1);

    if (roi.width > 0 && roi.height > 0) {
        cv::Mat zona = imgRedimensionada(roi);
        int k = spinFondo->value() | 1;
        cv::Mat fondo;
        cv::medianBlur(zona, fondo, k);

        cv::Mat lab, labFondo;
        cv::cvtColor(zona, lab, cv::COLOR_BGR2Lab);
        cv::cvtColor(fondo, labFondo, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> c, cf;
        cv::split(lab, c);
        cv::split(labFondo, cf);

        cv::Mat L, Lf, a, af;
        c[0].convertTo(L, CV_32F);
        cf[0].convertTo(Lf, CV_32F);
        c[1].convertTo(a, CV_32F);
        cf[1].convertTo(af, CV_32F);

        cv::Mat oscuro = (Lf - L) > spinUmbralOscuro->value();
        oscuro &= mascara_placa(roi);
        cv::morphologyEx(oscuro, oscuro, cv::MORPH_OPEN, cv::Mat::ones(2, 2, CV_8U));
        oscuro.copyTo(mascara_colonias(roi));

        cv::Mat(a - af).copyTo(deltaMagenta(roi));

        cv::Mat hsv;
        cv::cvtColor(zona, hsv, cv::COLOR_BGR2HSV);
        cv::Mat azul;
        cv::inRange(hsv, cv::Scalar(90, spinSatAzul->value(), 0), cv::Scalar(125, 255, 255), azul);
        azul.copyTo(mascaraAzul(roi));
    }

    std::vector<std::vector<cv::Point>> contornos;
    cv::findContours(mascara_colonias, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    if (chkDividirLinea->isChecked() && chkMostrarDiagnostico->isChecked()) {
        cv::line(mascaraDiagnostico, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(50, 50, 50), 1);
    }

    int violetasA = 0, violetasB = 0, azulesA = 0, azulesB = 0, ignoradas = 0;
    int zonaMuertaPx = spinZonaMuerta->value();
    double fraccionAzulMin = spinFraccionAzul->value() / 100.0;

    for (size_t i = 0; i < contornos.size(); i++) {
        int xMin = INT_MAX, yMin = INT_MAX, xMax = INT_MIN, yMax = INT_MIN;
        for (const cv::Point &p : contornos[i]) {
            xMin = std::min(xMin, p.x);
            yMin = std::min(yMin, p.y);
            xMax = std::max(xMax, p.x);
            yMax = std::max(yMax, p.y);
        }
        cv::Rect caja(xMin, yMin, xMax - xMin + 1, yMax - yMin + 1);
        cv::Mat mancha = cv::Mat::zeros(caja.size(), CV_8UC1);
        cv::drawContours(mancha, contornos, i, cv::Scalar(255), cv::FILLED, cv::LINE_8, cv::noArray(), 0, -caja.tl());
        int area = cv::countNonZero(mancha);

        if (area < spinAreaMin->value() || area > spinAreaMax->value()) {
            if (chkMostrarDiagnostico->isChecked()) {
                cv::Scalar color = area < spinAreaMin->value() ? cv::Scalar(150, 0, 150) : cv::Scalar(255, 255, 0);
                cv::drawContours(mascaraDiagnostico, contornos, i, color, cv::FILLED);
            }
            continue;
        }

        double fraccionAzul = cv::mean(mascaraAzul(caja), mancha)[0] / 255.0;
        double magenta = cv::mean(deltaMagenta(caja), mancha)[0];

        bool esAzul = fraccionAzul >= fraccionAzulMin && area >= spinAreaMinAzul->value();
        bool esVioleta = !esAzul && magenta > spinUmbralMagenta->value();

        if (!esAzul && !esVioleta) {
            if (chkMostrarDiagnostico->isChecked()) {
                cv::drawContours(mascaraDiagnostico, contornos, i, cv::Scalar(90, 60, 30), cv::FILLED);
            }
            ignoradas++;
            continue;
        }

        long sumaX = 0, sumaY = 0;
        for (int fy = 0; fy < mancha.rows; fy++) {
            const uchar *fila = mancha.ptr<uchar>(fy);
            for (int fx = 0; fx < mancha.cols; fx++) {
                if (fila[fx]) {
                    sumaX += fx;
                    sumaY += fy;
                }
            }
        }
        int cX = caja.x + static_cast<int>(sumaX / area);
        int cY = caja.y + static_cast<int>(sumaY / area);

        bool ladoA = false;
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

            double val_lado = (x1 - x2) * (cY - y2) - (y1 - y2) * (cX - x2);
            ladoA = val_lado > 0;
        }

        if (esAzul) {
            ladoA ? azulesA++ : azulesB++;
        } else {
            ladoA ? violetasA++ : violetasB++;
        }

        if (chkMostrarDiagnostico->isChecked()) {
            cv::Scalar color = esAzul ? cv::Scalar(0, 140, 255) : cv::Scalar(220, 0, 220);
            cv::drawContours(mascaraDiagnostico, contornos, i, color, cv::FILLED);
        }

        if (chkMostrarResultado->isChecked() && chkMostrarPuntos->isChecked()) {
            if (esAzul) {
                cv::circle(imgResultado, cv::Point(cX, cY), 9, cv::Scalar(255, 255, 0), 2);
            } else {
                cv::circle(imgResultado, cv::Point(cX, cY), 4, cv::Scalar(0, 255, 0), 1);
            }
        }
    }

    int violetas = violetasA + violetasB;
    int azules = azulesA + azulesB;
    totalColoniasContadas = violetas + azules;

    qDebug() << "[RESULTADOS] Violetas:" << violetas << "| Azules:" << azules << "| Ignoradas:" << ignoradas;

    if (chkDividirLinea->isChecked()) {
        lblColoniasA->setText(QString("Colonias violetas: %1 (A: %2 | B: %3)").arg(violetas).arg(violetasA).arg(violetasB));
        lblColoniasB->setText(QString("Colonias azules: %1 (A: %2 | B: %3)").arg(azules).arg(azulesA).arg(azulesB));
    } else {
        lblColoniasA->setText("Colonias violetas: " + QString::number(violetas));
        lblColoniasB->setText("Colonias azules: " + QString::number(azules));
    }

    imgProcesada = imgRedimensionada;
    hayCirculo = !circulos.empty();
    circuloPlaca = cv::Vec3i(x_circulo, y_circulo, r_circulo);
    rectPlaca = cv::Rect(0, 0, imgRedimensionada.cols, imgRedimensionada.rows);
    if (hayCirculo) {
        rectPlaca &= cv::Rect(x_circulo - r_circulo - 4, y_circulo - r_circulo - 4, 2 * (r_circulo + 4), 2 * (r_circulo + 4));
    }

    // --- RENDERIZADO EN UI ---
    lblSinImagen->hide();
    acomodarCapas();

    cv::Rect vista(0, 0, imgRedimensionada.cols, imgRedimensionada.rows);
    if (chkRecortarPlaca->isChecked() && !circulos.empty()) {
        int m = 4;
        vista &= cv::Rect(x_circulo - r_circulo - m, y_circulo - r_circulo - m, 2 * (r_circulo + m), 2 * (r_circulo + m));
    }

    if (chkMostrarOriginal->isChecked()) {
        mostrarImagen(lblImagenOriginal, matToQImage(imgRGB(vista)), "1. Imagen Original");
    } else {
        mostrarDesactivada(lblImagenOriginal, "Imagen Original", QSize(vista.width, vista.height));
    }

    if (chkMostrarDiagnostico->isChecked()) {
        mostrarImagen(lblImagenDiagnostico, matToQImage(mascaraDiagnostico(vista)), "2. Mapa de Diagnóstico");
    } else {
        mostrarDesactivada(lblImagenDiagnostico, "Mapa de Diagnóstico", QSize(vista.width, vista.height));
    }

    if (chkMostrarRGB->isChecked()) {
        mostrarImagen(lblImagenParametrica, matToQImage(mascara_colonias(vista)), "3. Máscara de Manchas");
    } else {
        mostrarDesactivada(lblImagenParametrica, "Máscara de Manchas", QSize(vista.width, vista.height));
    }

    if (chkMostrarResultado->isChecked()) {
        mostrarImagen(lblImagenResultado, matToQImage(imgResultado(vista)), "4. Resultado Final");
    } else {
        mostrarDesactivada(lblImagenResultado, "Resultado Final", QSize(vista.width, vista.height));
    }
}

void MainWindow::agregarParametro(QGridLayout *layout, const QString &nombre, QSpinBox *spin) {
    QSlider *slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(spin->minimum(), spin->maximum());
    slider->setValue(spin->value());
    slider->setTracking(false);

    connect(slider, &QSlider::sliderMoved, this, [spin](int valor) {
        QSignalBlocker bloqueo(spin);
        spin->setValue(valor);
    });
    connect(slider, &QSlider::valueChanged, this, [this, spin](int valor) {
        if (spin->value() == valor) {
            procesarImagen();
        } else {
            spin->setValue(valor);
        }
    });
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slider, [slider](int valor) {
        QSignalBlocker bloqueo(slider);
        slider->setValue(valor);
    });

    int fila = layout->rowCount();
    layout->addWidget(new QLabel(nombre, this), fila, 0);
    layout->addWidget(slider, fila, 1);
    layout->addWidget(spin, fila, 2);
}

void MainWindow::restaurarValores() {
    pausarProceso = true;
    for (QSpinBox *spin : findChildren<QSpinBox *>()) {
        if (spin->property("defecto").isValid()) {
            spin->setValue(spin->property("defecto").toInt());
        }
    }
    pausarProceso = false;
    procesarImagen();
}

void MainWindow::mostrarImagen(QLabel *destino, const QImage &imagen, const QString &titulo) {
    QPixmap pixmap = QPixmap::fromImage(imagen).scaled(destino->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter pintor(&pixmap);
    QFont fuente = font();
    fuente.setBold(true);
    fuente.setPixelSize(14);
    pintor.setFont(fuente);
    QRect caja = pintor.fontMetrics().boundingRect(titulo).adjusted(-6, -3, 6, 3);
    caja.moveTopLeft(QPoint(4, 4));
    pintor.fillRect(caja, QColor(0, 0, 0, 150));
    pintor.setPen(Qt::white);
    pintor.drawText(caja, Qt::AlignCenter, titulo);
    pintor.end();

    destino->setAlignment(Qt::Alignment(destino->property("alineacion").toInt()));
    destino->setPixmap(pixmap);
}

void MainWindow::acomodarCapas() {
    QGridLayout *grilla = qobject_cast<QGridLayout *>(contenedorImagenes->layout());
    QList<QLabel *> capas = {lblImagenOriginal, lblImagenDiagnostico, lblImagenParametrica, lblImagenResultado};
    QList<QCheckBox *> checks = {chkMostrarOriginal, chkMostrarDiagnostico, chkMostrarRGB, chkMostrarResultado};
    QList<int> alineaciones = {int(Qt::AlignRight | Qt::AlignBottom), int(Qt::AlignLeft | Qt::AlignBottom),
                               int(Qt::AlignRight | Qt::AlignTop), int(Qt::AlignLeft | Qt::AlignTop)};

    int activas = 0;
    for (QCheckBox *chk : checks) {
        if (chk->isChecked()) activas++;
    }

    for (int i = 0; i < capas.size(); i++) {
        grilla->removeWidget(capas[i]);
        if (activas == 1) {
            if (checks[i]->isChecked()) {
                capas[i]->setProperty("alineacion", int(Qt::AlignCenter));
                grilla->addWidget(capas[i], 0, 0, 2, 2);
                capas[i]->show();
            } else {
                capas[i]->hide();
            }
        } else {
            capas[i]->setProperty("alineacion", alineaciones[i]);
            grilla->addWidget(capas[i], i / 2, i % 2);
            capas[i]->show();
        }
    }
    grilla->activate();
}

void MainWindow::buscarActualizacion() {
    QNetworkAccessManager *red = new QNetworkAccessManager(this);
    QNetworkRequest pedido(QUrl("https://api.github.com/repos/Tmas-mnslla-pnzzo/contador-colonias/releases/latest"));
    pedido.setHeader(QNetworkRequest::UserAgentHeader, "ContadorColonias/" APP_VERSION);
    pedido.setTransferTimeout(8000);
    QNetworkReply *respuesta = red->get(pedido);

    connect(respuesta, &QNetworkReply::finished, this, [this, red, respuesta]() {
        respuesta->deleteLater();
        red->deleteLater();
        if (respuesta->error() != QNetworkReply::NoError) return;

        QJsonObject datos = QJsonDocument::fromJson(respuesta->readAll()).object();
        QString etiqueta = datos.value("tag_name").toString();
        if (etiqueta.startsWith('v')) etiqueta.remove(0, 1);

        QVersionNumber nueva = QVersionNumber::fromString(etiqueta);
        QVersionNumber actual = QVersionNumber::fromString(APP_VERSION);
        if (nueva.isNull() || nueva <= actual) return;

        QString enlace = datos.value("html_url").toString();
        QMessageBox aviso(this);
        aviso.setWindowTitle("Actualización disponible");
        aviso.setIcon(QMessageBox::Information);
        aviso.setTextFormat(Qt::RichText);
        aviso.setTextInteractionFlags(Qt::TextBrowserInteraction);
        aviso.setText(QString("Hay una versión nueva del programa: <b>%1</b> (versión instalada: %2).<br><br>"
                              "<a href=\"%3\">Descargar la versión %1</a><br><br>"
                              "En la página, bajar el archivo <i>InstaladorContadorColonias</i> y ejecutarlo.")
                          .arg(etiqueta, APP_VERSION, enlace));
        aviso.exec();
    });
}

void MainWindow::abrirAutocalibracion() {
    if (imgProcesada.empty()) {
        QMessageBox::information(this, "Autocalibración", "Primero hay que cargar una imagen.");
        return;
    }

    cv::Mat recorte;
    cv::cvtColor(imgProcesada(rectPlaca), recorte, cv::COLOR_BGR2RGB);
    if (hayCirculo) {
        cv::circle(recorte, cv::Point(circuloPlaca[0] - rectPlaca.x, circuloPlaca[1] - rectPlaca.y), circuloPlaca[2],
                   cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    }

    DialogoCalibracion dialogo(matToQImage(recorte), this);
    if (dialogo.exec() != QDialog::Accepted) return;

    QVector<Marca> marcas = dialogo.marcas();
    for (Marca &m : marcas) {
        m.punto += QPoint(rectPlaca.x, rectPlaca.y);
    }
    calibrar(marcas);
}

void MainWindow::calibrar(const QVector<Marca> &marcas) {
    cv::Mat fondo;
    cv::medianBlur(imgProcesada, fondo, spinFondo->value() | 1);

    cv::Mat lab, labFondo, hsv;
    cv::cvtColor(imgProcesada, lab, cv::COLOR_BGR2Lab);
    cv::cvtColor(fondo, labFondo, cv::COLOR_BGR2Lab);
    cv::cvtColor(imgProcesada, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> c, cf, ch;
    cv::split(lab, c);
    cv::split(labFondo, cf);
    cv::split(hsv, ch);

    cv::Mat L, Lf, a, af;
    c[0].convertTo(L, CV_32F);
    cf[0].convertTo(Lf, CV_32F);
    c[1].convertTo(a, CV_32F);
    cf[1].convertTo(af, CV_32F);
    cv::Mat oscuridad = Lf - L;
    cv::Mat deltaMagenta = a - af;
    cv::Rect imagenCompleta(0, 0, imgProcesada.cols, imgProcesada.rows);

    auto picoCercano = [&](QPoint p, int radio) {
        cv::Rect zona = cv::Rect(p.x() - radio, p.y() - radio, 2 * radio + 1, 2 * radio + 1) & imagenCompleta;
        double maximo;
        cv::Point posicion;
        cv::minMaxLoc(oscuridad(zona), nullptr, &maximo, nullptr, &posicion);
        return std::make_pair(posicion + zona.tl(), float(maximo));
    };

    std::vector<cv::Point> picos;
    std::vector<float> valoresPico, picosColonia, ruidoFondo;
    for (const Marca &m : marcas) {
        auto [posicion, valor] = picoCercano(m.punto, 4);
        picos.push_back(posicion);
        valoresPico.push_back(valor);
        if (m.tipo == MarcaFondo) {
            ruidoFondo.push_back(valor);
        } else {
            picosColonia.push_back(valor);
        }
    }

    std::vector<float> ordenados = picosColonia;
    std::nth_element(ordenados.begin(), ordenados.begin() + ordenados.size() / 2, ordenados.end());
    float medianaColonia = ordenados[ordenados.size() / 2];
    std::vector<bool> descartada(marcas.size(), false);
    int descartadas = 0;
    picosColonia.clear();
    for (int i = 0; i < marcas.size(); i++) {
        if (marcas[i].tipo == MarcaFondo) continue;
        if (valoresPico[i] < 0.4f * medianaColonia) {
            descartada[i] = true;
            descartadas++;
        } else {
            picosColonia.push_back(valoresPico[i]);
        }
    }

    float minimoColonia = *std::min_element(picosColonia.begin(), picosColonia.end());
    float maximoFondo = *std::max_element(ruidoFondo.begin(), ruidoFondo.end());
    bool separables = minimoColonia > maximoFondo + 1;
    int umbralOscuro = separables ? cvRound((minimoColonia + maximoFondo) / 2) : cvRound(minimoColonia - 1);
    umbralOscuro = std::clamp(umbralOscuro, 8, 100);

    struct Region { int area; double magenta; std::vector<int> saturaciones; std::vector<int> tonos; };
    auto regionDe = [&](cv::Point pico) {
        Region r{1, deltaMagenta.at<float>(pico), {ch[1].at<uchar>(pico)}, {ch[0].at<uchar>(pico)}};
        cv::Rect zona = cv::Rect(pico.x - 30, pico.y - 30, 61, 61) & imagenCompleta;
        cv::Mat mascara = oscuridad(zona) > umbralOscuro;
        cv::Point local = pico - zona.tl();
        if (!mascara.at<uchar>(local)) return r;

        cv::Mat etiquetas;
        cv::connectedComponents(mascara, etiquetas, 8, CV_32S);
        int etiqueta = etiquetas.at<int>(local);
        r = Region{0, 0, {}, {}};
        for (int y = 0; y < etiquetas.rows; y++) {
            for (int x = 0; x < etiquetas.cols; x++) {
                if (etiquetas.at<int>(y, x) != etiqueta) continue;
                cv::Point g = cv::Point(x, y) + zona.tl();
                r.area++;
                r.magenta += deltaMagenta.at<float>(g);
                r.saturaciones.push_back(ch[1].at<uchar>(g));
                r.tonos.push_back(ch[0].at<uchar>(g));
            }
        }
        r.magenta /= r.area;
        return r;
    };

    std::vector<Region> violetas, azules;
    for (int i = 0; i < marcas.size(); i++) {
        if (descartada[i]) continue;
        if (marcas[i].tipo == MarcaVioleta) violetas.push_back(regionDe(picos[i]));
        else if (marcas[i].tipo == MarcaAzul) azules.push_back(regionDe(picos[i]));
    }

    bool calibrarVioletas = std::any_of(marcas.begin(), marcas.end(), [](const Marca &m) { return m.tipo == MarcaVioleta; });
    bool calibrarAzules = std::any_of(marcas.begin(), marcas.end(), [](const Marca &m) { return m.tipo == MarcaAzul; });
    if ((calibrarVioletas && violetas.size() < 2) || (calibrarAzules && azules.size() < 2)) {
        QMessageBox::warning(this, "Autocalibración",
                             "Quedaron muy pocas marcas válidas de colonias violetas o azules. "
                             "No se cambió ningún valor. Volver a marcar sobre colonias bien visibles.");
        return;
    }

    int umbralMagenta = spinUmbralMagenta->value();
    if (calibrarVioletas) {
        double minMagenta = 1e9;
        for (const Region &r : violetas) minMagenta = std::min(minMagenta, r.magenta);
        umbralMagenta = std::clamp(int(std::floor(minMagenta)) - 2, -20, 30);
    }

    int satAzul = spinSatAzul->value();
    int fraccionAzul = spinFraccionAzul->value();
    int areaMinAzul = spinAreaMinAzul->value();
    if (calibrarAzules) {
        double minSaturacionAzul = 255;
        for (Region &r : azules) {
            std::vector<int> s = r.saturaciones;
            std::nth_element(s.begin(), s.begin() + s.size() / 2, s.end());
            minSaturacionAzul = std::min(minSaturacionAzul, double(s[s.size() / 2]));
        }
        satAzul = std::clamp(int(minSaturacionAzul * 0.6), 0, 255);

        auto fraccionAzulDe = [&](const Region &r) {
            int n = 0;
            for (size_t i = 0; i < r.tonos.size(); i++) {
                if (r.tonos[i] >= 90 && r.tonos[i] <= 125 && r.saturaciones[i] >= satAzul) n++;
            }
            return double(n) / r.tonos.size();
        };
        double minFraccionAzul = 1;
        for (const Region &r : azules) minFraccionAzul = std::min(minFraccionAzul, fraccionAzulDe(r));
        fraccionAzul = cvRound(minFraccionAzul * 50);
        fraccionAzul = std::clamp(fraccionAzul, 5, 95);

        int minAreaAzul = INT_MAX;
        for (const Region &r : azules) minAreaAzul = std::min(minAreaAzul, r.area);
        areaMinAzul = std::max(1, int(minAreaAzul * 0.3));
    }

    int minArea = INT_MAX, maxArea = 0;
    for (const auto *grupo : {&violetas, &azules}) {
        for (const Region &r : *grupo) {
            minArea = std::min(minArea, r.area);
            maxArea = std::max(maxArea, r.area);
        }
    }
    int areaMin = std::max(1, int(minArea * 0.7));
    int areaMax = std::clamp(int(std::ceil(maxArea * 1.5)), std::max(areaMin + 1, 50), 5000);

    pausarProceso = true;
    spinUmbralOscuro->setValue(umbralOscuro);
    spinUmbralMagenta->setValue(umbralMagenta);
    spinSatAzul->setValue(satAzul);
    spinFraccionAzul->setValue(fraccionAzul);
    spinAreaMinAzul->setValue(areaMinAzul);
    spinAreaMin->setValue(areaMin);
    spinAreaMax->setValue(areaMax);
    pausarProceso = false;
    procesarImagen();

    QString resumen = QString("Valores calculados a partir de las marcas:<br><br>"
                              "Umbral oscuro: %1<br>Umbral violeta: %2<br>Saturación azul: %3<br>"
                              "% azul: %4<br>Área mín. azul: %5<br>Área mínima: %6<br>Área máxima: %7<br><br>"
                              "Para conservarlos al reabrir el programa, usar <i>Guardar config.</i>")
                          .arg(umbralOscuro).arg(umbralMagenta).arg(satAzul).arg(fraccionAzul)
                          .arg(areaMinAzul).arg(areaMin).arg(areaMax);
    if (!calibrarVioletas) {
        resumen += "<br><br>No se marcaron violetas: el umbral violeta quedó como estaba.";
    }
    if (!calibrarAzules) {
        resumen += "<br><br>No se marcaron azules: saturación azul, % azul y área mín. azul quedaron como estaban.";
    }
    if (descartadas > 0) {
        resumen += QString("<br><br><b>Atención:</b> se descartaron %1 marca(s) de colonia porque son mucho más claras "
                           "que el resto y probablemente no sean colonias.").arg(descartadas);
    }
    if (!separables) {
        resumen += "<br><br><b>Atención:</b> alguna marca de fondo es tan oscura como la colonia más clara. "
                   "Revisar que las marcas de fondo estén sobre zonas limpias.";
    }
    QMessageBox::information(this, "Autocalibración aplicada", resumen);
}

void MainWindow::mostrarDesactivada(QLabel *destino, const QString &nombre, const QSize &tamImagen) {
    QSize tam = tamImagen.scaled(destino->size(), Qt::KeepAspectRatio);
    QPixmap pixmap(tam);
    pixmap.fill(palette().color(QPalette::Window));

    QPainter pintor(&pixmap);
    pintor.setPen(QPen(palette().color(QPalette::Mid), 1, Qt::DashLine));
    pintor.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));
    pintor.setPen(palette().color(QPalette::WindowText));
    pintor.drawText(pixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, "Capa " + nombre + " desactivada");
    pintor.end();

    destino->setAlignment(Qt::Alignment(destino->property("alineacion").toInt()));
    destino->setPixmap(pixmap);
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

    settings.setValue("fondo", spinFondo->value());
    settings.setValue("umbralOscuro", spinUmbralOscuro->value());
    settings.setValue("umbralMagenta", spinUmbralMagenta->value());
    settings.setValue("satAzul", spinSatAzul->value());
    settings.setValue("areaMinAzul", spinAreaMinAzul->value());
    settings.setValue("fraccionAzul", spinFraccionAzul->value());

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

    spinFondo->setValue(settings.value("fondo", 31).toInt());
    spinUmbralOscuro->setValue(settings.value("umbralOscuro", 25).toInt());
    spinUmbralMagenta->setValue(settings.value("umbralMagenta", 0).toInt());
    spinSatAzul->setValue(settings.value("satAzul", 45).toInt());
    spinAreaMinAzul->setValue(settings.value("areaMinAzul", 15).toInt());
    spinFraccionAzul->setValue(settings.value("fraccionAzul", 30).toInt());

    spinAreaMin->setValue(settings.value("areaMin", 2).toInt());
    spinAreaMax->setValue(settings.value("areaMax", 800).toInt());
}

QImage MainWindow::componerImagenGuardado() {
    QPixmap captura = contenedorImagenes->grab();
    qreal dpr = captura.devicePixelRatio();
    int anchoL = qRound(captura.width() / dpr);
    int altoL = qRound(captura.height() / dpr);

    QStringList datos;
    datos << QString("Tipo: %1     ID de placa: %2     Fecha: %3")
                 .arg(comboBacterias->currentText(),
                      txtIdPlaca->text().trimmed().isEmpty() ? "-" : txtIdPlaca->text().trimmed(),
                      QDate::currentDate().toString("dd/MM/yyyy"));
    datos << lblInfoNombre->text().replace("Nombre:", "Imagen:");
    datos << lblColoniasA->text();
    datos << lblColoniasB->text();
    datos << QString("Total de colonias: %1").arg(totalColoniasContadas);

    QFont fuente = font();
    fuente.setPixelSize(16);
    QFontMetrics metricas(fuente);
    int margen = 12;
    int altoLinea = metricas.height() + 4;
    int banda = datos.size() * altoLinea + 2 * margen;

    QImage salida(QSize(anchoL, altoL + banda) * dpr, QImage::Format_RGB32);
    salida.setDevicePixelRatio(dpr);
    salida.fill(Qt::white);

    QPainter pintor(&salida);
    pintor.setRenderHint(QPainter::TextAntialiasing);
    pintor.setPen(Qt::black);
    for (int i = 0; i < datos.size(); i++) {
        fuente.setBold(i >= 2);
        pintor.setFont(fuente);
        pintor.drawText(margen, margen + i * altoLinea + metricas.ascent(), datos[i]);
    }
    int anchoTexto = 0;
    for (int i = 0; i < datos.size(); i++) {
        fuente.setBold(i >= 2);
        anchoTexto = std::max(anchoTexto, QFontMetrics(fuente).horizontalAdvance(datos[i]));
    }
    QImage logo(":/logo_fcal.png");
    if (!logo.isNull()) {
        QSize lugar(anchoL - anchoTexto - 3 * margen, std::min(80, banda - 2 * margen));
        if (lugar.width() > 0 && lugar.height() > 0) {
            QSize tamLogo = logo.size().scaled(lugar, Qt::KeepAspectRatio);
            QRect destinoLogo(QPoint(anchoL - margen - tamLogo.width(), margen), tamLogo);
            pintor.setRenderHint(QPainter::SmoothPixmapTransform);
            pintor.drawImage(destinoLogo, logo);
        }
    }

    pintor.setPen(QColor(180, 180, 180));
    pintor.drawLine(0, banda - 1, anchoL, banda - 1);
    pintor.drawPixmap(0, banda, captura);
    pintor.end();

    return salida;
}

void MainWindow::guardarImagenLayout() {
    if (imagenOriginal.empty()) {
        QMessageBox::warning(this, "Aviso", "No hay ninguna imagen procesada para guardar.");
        return;
    }

    // 1. Obtener la fecha en formato yyMMdd (ej. 260705 para el 5 de julio de 2026)
    QString fechaActual = QDate::currentDate().toString("yyMMdd");

    // 2. Obtener el nombre de la bacteria y pasarlo a minúsculas
    QString bacteria;
    for (const QChar &ch : comboBacterias->currentText().toLower().normalized(QString::NormalizationForm_D)) {
        if (ch.isLetterOrNumber() && ch.unicode() < 128) bacteria += ch;
        else if (ch.isSpace()) bacteria += '_';
    }
    bacteria += '_';

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
        QImage salida = componerImagenGuardado();

        if (salida.save(rutaGuardado)) {
            qDebug() << "[LOG] Layout guardado en:" << rutaGuardado;
            QMessageBox::information(this, "Éxito", "Imagen guardada correctamente.");
        } else {
            QMessageBox::critical(this, "Error", "No se pudo guardar la imagen.");
        }
    }
}
void MainWindow::mostrarInformacion() {
    QString textoInfo =
        "<h2>Análisis de Placas de Petri v" APP_VERSION "</h2>"
        "<p>Software paramétrico de visión artificial para el conteo automático de unidades formadoras de colonias.</p>"
        "<hr>"
        "<p><b>Desarrollado para:</b><br>"
        "Laboratorio de Microbiología y Biotecnología de Alimentos (MIBIAL)<br>"
        "Facultad de Ciencias de la Alimentación - UNER</p>"
        "<p><b>Motor de Procesamiento:</b> OpenCV y Qt6 C++</p>"
        "<p><b>Ícono:</b> OpenMoji (openmoji.org), licencia CC BY-SA 4.0</p>"
        "<p><b>Uso de Parámetros:</b> agrupados en las pestañas División, Detección y Placa (avanzado). El botón <i>Restaurar valores</i> vuelve a los valores de fábrica.<br>"
                        "- <i>Ancho proceso:</i> Resolución interna de cálculo. <b>Recomendación:</b> No cambiar. Alterarlo desconfigura la escala de radios y áreas.<br>"
                        "- <i>Ángulo división:</i> Inclinación de la recta separadora. <b>Recomendación:</b> Ajustar en cada foto para alinear con la división física de la placa.<br>"
                        "- <i>Zona muerta:</i> Margen ignorado en la línea divisoria. <b>Recomendación:</b> Ajustar solo si hay ruido o bacterias pisando la raya.<br>"
                        "- <i>Radio mínimo / máximo:</i> Tamaño del círculo de la placa en píxeles. <b>Recomendación:</b> Cambiar solo si la cámara se acercó o alejó mucho.<br>"
                        "- <i>Hough 1 y 2:</i> Sensibilidad matemática del detector de la placa. <b>Recomendación:</b> No tocar a menos que el círculo verde falle.<br>"
                        "- <i>Reducción borde:</i> Margen interno para recortar la imagen. <b>Recomendación:</b> Aumentar si el brillo del borde del plástico cuenta como falsas bacterias.<br>"
                        "- <i>Fondo (px):</i> Tamaño de la ventana con que se estima el fondo de la placa. <b>Recomendación:</b> Debe ser bastante mayor que la colonia más grande; subirlo si las colonias azules salen huecas.<br>"
                        "- <i>Umbral oscuro:</i> Cuánto más oscura que el fondo tiene que ser una mancha para tomarse en cuenta. <b>Recomendación:</b> Bajarlo si quedan puntitos violetas sin marcar; subirlo si aparece la textura de la placa.<br>"
                        "- <i>Umbral violeta:</i> Cuánto más rojiza que el fondo tiene que ser la mancha para contarse como violeta. <b>Recomendación:</b> Subirlo si se cuentan puntos grises o polvo.<br>"
                        "- <i>Saturación azul / % azul / Área mín. azul:</i> Criterio para clasificar una mancha como colonia azul. <b>Recomendación:</b> Bajar Saturación azul o % azul si alguna azul se marca como violeta.<br>"
                        "- <i>Área mínima / máxima:</i> Filtro de tamaño geométrico. <b>Recomendación:</b> Subir la mínima para ignorar polvo o ruido; bajar la máxima para ignorar grietas o reflejos grandes.</p>"
        "<p><b>Mapa de diagnóstico:</b> magenta = violeta contada, celeste = azul contada, marrón = mancha sin color de colonia, gris = zona muerta, violeta oscuro = muy chica, amarillo = muy grande.</p>";

    // El QMessageBox::about es especial para estas ventanas de información institucional
    QMessageBox::about(this, "Acerca del Sistema", textoInfo);
}