#include "calibracion.h"
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <cmath>

static QColor colorMarca(TipoMarca tipo) {
    switch (tipo) {
    case MarcaVioleta: return QColor(230, 0, 230);
    case MarcaAzul: return QColor(0, 200, 255);
    default: return QColor(255, 210, 0);
    }
}

LienzoCalibracion::LienzoCalibracion(const QImage &imagen, QWidget *parent)
    : QWidget(parent), imagen(imagen) {
    setZoom(1.0);
    setCursor(Qt::CrossCursor);
}

void LienzoCalibracion::setZoom(double zoom) {
    factorZoom = std::clamp(zoom, 0.25, 8.0);
    setFixedSize(imagen.size() * factorZoom);
    update();
}

void LienzoCalibracion::deshacer() {
    if (listaMarcas.isEmpty()) return;
    listaMarcas.removeLast();
    update();
    emit marcasCambiadas();
}

void LienzoCalibracion::paintEvent(QPaintEvent *) {
    QPainter pintor(this);
    pintor.setRenderHint(QPainter::SmoothPixmapTransform);
    pintor.drawImage(rect(), imagen);
    pintor.setRenderHint(QPainter::Antialiasing);

    for (const Marca &m : listaMarcas) {
        QPointF c = QPointF(m.punto.x() + 0.5, m.punto.y() + 0.5) * factorZoom;
        pintor.setPen(QPen(Qt::black, 4));
        pintor.setBrush(Qt::NoBrush);
        if (m.tipo == MarcaFondo) {
            pintor.drawLine(c + QPointF(-7, -7), c + QPointF(7, 7));
            pintor.drawLine(c + QPointF(-7, 7), c + QPointF(7, -7));
            pintor.setPen(QPen(colorMarca(m.tipo), 2));
            pintor.drawLine(c + QPointF(-7, -7), c + QPointF(7, 7));
            pintor.drawLine(c + QPointF(-7, 7), c + QPointF(7, -7));
        } else {
            pintor.drawEllipse(c, 9, 9);
            pintor.setPen(QPen(colorMarca(m.tipo), 2));
            pintor.drawEllipse(c, 9, 9);
        }
    }
}

void LienzoCalibracion::mousePressEvent(QMouseEvent *event) {
    QPointF pos = event->position();

    if (event->button() == Qt::RightButton) {
        int masCercana = -1;
        double distanciaMin = 14;
        for (int i = 0; i < listaMarcas.size(); i++) {
            QPointF c = QPointF(listaMarcas[i].punto) * factorZoom;
            double d = std::hypot(c.x() - pos.x(), c.y() - pos.y());
            if (d < distanciaMin) {
                distanciaMin = d;
                masCercana = i;
            }
        }
        if (masCercana >= 0) {
            listaMarcas.removeAt(masCercana);
            update();
            emit marcasCambiadas();
        }
        return;
    }

    if (event->button() != Qt::LeftButton) return;

    QPoint punto(int(pos.x() / factorZoom), int(pos.y() / factorZoom));
    if (!imagen.rect().contains(punto)) return;

    listaMarcas.append({tipoActual, punto});
    update();
    emit marcasCambiadas();
}

DialogoCalibracion::DialogoCalibracion(const QImage &imagen, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Autocalibración");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setSizeGripEnabled(true);
    resize(900, 850);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *lblAyuda = new QLabel(
        "<b>Marcar como mínimo 3 de fondo limpio y 3 de cada color que se quiera calibrar</b> "
        "(solo violetas, solo azules o ambos).<br>"
        "Elegir el tipo con los botones y hacer clic sobre la imagen. Clic derecho sobre una marca la borra.<br>"
        "Conviene incluir las colonias más chicas y claras, no solo las más visibles.", this);
    lblAyuda->setWordWrap(true);
    layout->addWidget(lblAyuda);

    QHBoxLayout *barra = new QHBoxLayout();
    btnVioleta = new QPushButton(this);
    btnAzul = new QPushButton(this);
    btnFondo = new QPushButton(this);
    QButtonGroup *grupo = new QButtonGroup(this);
    for (QPushButton *b : {btnVioleta, btnAzul, btnFondo}) {
        b->setCheckable(true);
        grupo->addButton(b);
        barra->addWidget(b);
    }
    btnVioleta->setChecked(true);

    barra->addStretch();
    QPushButton *btnAchicar = new QPushButton("Zoom −", this);
    QPushButton *btnAgrandar = new QPushButton("Zoom +", this);
    QPushButton *btnDeshacer = new QPushButton("Deshacer", this);
    barra->addWidget(btnAchicar);
    barra->addWidget(btnAgrandar);
    barra->addWidget(btnDeshacer);
    layout->addLayout(barra);

    lienzo = new LienzoCalibracion(imagen, this);
    scroll = new QScrollArea(this);
    scroll->setWidget(lienzo);
    scroll->setAlignment(Qt::AlignCenter);
    layout->addWidget(scroll, 1);

    lblEstado = new QLabel(this);
    layout->addWidget(lblEstado);

    botones = new QDialogButtonBox(this);
    botones->addButton("Aplicar calibración", QDialogButtonBox::AcceptRole);
    botones->addButton("Cancelar", QDialogButtonBox::RejectRole);
    layout->addWidget(botones);

    connect(btnVioleta, &QPushButton::clicked, this, [this]() { lienzo->setTipo(MarcaVioleta); });
    connect(btnAzul, &QPushButton::clicked, this, [this]() { lienzo->setTipo(MarcaAzul); });
    connect(btnFondo, &QPushButton::clicked, this, [this]() { lienzo->setTipo(MarcaFondo); });

    auto cambiarZoom = [this](double factor) {
        QScrollBar *h = scroll->horizontalScrollBar();
        QScrollBar *v = scroll->verticalScrollBar();
        double cx = (h->value() + scroll->viewport()->width() / 2.0) / lienzo->zoom();
        double cy = (v->value() + scroll->viewport()->height() / 2.0) / lienzo->zoom();
        lienzo->setZoom(lienzo->zoom() * factor);
        h->setValue(int(cx * lienzo->zoom() - scroll->viewport()->width() / 2.0));
        v->setValue(int(cy * lienzo->zoom() - scroll->viewport()->height() / 2.0));
    };
    connect(btnAgrandar, &QPushButton::clicked, this, [cambiarZoom]() { cambiarZoom(1.25); });
    connect(btnAchicar, &QPushButton::clicked, this, [cambiarZoom]() { cambiarZoom(0.8); });
    connect(btnDeshacer, &QPushButton::clicked, lienzo, &LienzoCalibracion::deshacer);
    connect(lienzo, &LienzoCalibracion::marcasCambiadas, this, &DialogoCalibracion::actualizarEstado);
    connect(botones, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);

    double ajuste = std::min(820.0 / imagen.width(), 640.0 / imagen.height());
    lienzo->setZoom(std::max(1.0, ajuste));

    actualizarEstado();
}

void DialogoCalibracion::actualizarEstado() {
    int violetas = 0, azules = 0, fondo = 0;
    for (const Marca &m : lienzo->marcas()) {
        if (m.tipo == MarcaVioleta) violetas++;
        else if (m.tipo == MarcaAzul) azules++;
        else fondo++;
    }

    btnVioleta->setText(QString("● Violetas (%1)").arg(violetas));
    btnAzul->setText(QString("● Azules (%1)").arg(azules));
    btnFondo->setText(QString("✕ Fondo (%1)").arg(fondo));

    bool violetasOk = violetas == 0 || violetas >= 3;
    bool azulesOk = azules == 0 || azules >= 3;
    bool listo = fondo >= 3 && violetas + azules > 0 && violetasOk && azulesOk;
    botones->buttons().first()->setEnabled(listo);

    if (listo) {
        QString colores = violetas > 0 && azules > 0 ? "violetas y azules" : violetas > 0 ? "solo violetas" : "solo azules";
        lblEstado->setText(QString("Listo para aplicar (%1). Se pueden seguir agregando marcas para afinar.").arg(colores));
    } else {
        QStringList faltan;
        if (violetas + azules == 0) faltan << "3 colonias violetas y/o 3 azules";
        if (!violetasOk) faltan << QString("%1 violetas").arg(3 - violetas);
        if (!azulesOk) faltan << QString("%1 azules").arg(3 - azules);
        if (fondo < 3) faltan << QString("%1 de fondo").arg(3 - fondo);
        lblEstado->setText("Faltan: " + faltan.join(", ") + ".");
    }
}
