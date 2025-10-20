#pragma once
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QScrollArea>
#include <QToolBar>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QGraphicsDropShadowEffect>
#include <QDragEnterEvent>
#include <QPropertyAnimation>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>

#include <stack>
#include <unordered_map>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1400, 800);
    setWindowTitle("Wake Up At Dawn");
    setWindowIcon(QIcon("./icons/app_icon.ico"));
    setAcceptDrops(true);

    // 🎨 تحميل ملف QSS
    QFile styleFile("./styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        qApp->setStyleSheet(style);
    }

    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    setCentralWidget(central);

    setupToolbar();

    statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);

    // --------------------------------------------
    // 🧩 لوحة الفلاتر
    QFrame *filtersFrame = new QFrame;
    filtersFrame->setObjectName("frameFilters");
    filtersFrame->setMinimumWidth(270);
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersFrame);

    QLabel *filtersTitle = new QLabel("Filters");
    filtersTitle->setObjectName("panelTitle");
    filtersLayout->addWidget(filtersTitle);

    QScrollArea *filtersScroll = new QScrollArea;
    filtersScroll->setWidgetResizable(true);
    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);

    filters = {
               {GreyScale::getId(), make_shared<GreyScale>(customImage)},
               {WhiteAndBlack::getId(), make_shared<WhiteAndBlack>(customImage)},
               {Invert::getId(), make_shared<Invert>(customImage)},
               {Flip::getId(), make_shared<Flip>(customImage)},
               {Rotate::getId(), make_shared<Rotate>(customImage)},
               {Brightness::getId(), make_shared<Brightness>(customImage)},
               {Crop::getId(), make_shared<Crop>(customImage)},
               {Frame::getId(), make_shared<Frame>(customImage)},
               {EdgeDetection::getId(), make_shared<EdgeDetection>(customImage)},
               {Resize::getId(), make_shared<Resize>(customImage)},
               {Blur::getId(), make_shared<Blur>(customImage)},
               {Sunlight::getId(), make_shared<Sunlight>(customImage)},
               {OilPainting::getId(), make_shared<OilPainting>(customImage)},
               {OldTV::getId(), make_shared<OldTV>(customImage)},
               {Night::getId(), make_shared<Night>(customImage)},
               {Infrared::getId(), make_shared<Infrared>(customImage)},
               {Skewing::getId(), make_shared<Skewing>(customImage)},
               {Bloody::getId(), make_shared<Bloody>(customImage)},
               {Grass::getId(), make_shared<Grass>(customImage)},
               {Sky::getId(), make_shared<Sky>(customImage)},
               {ArtisticBrush::getId(), make_shared<ArtisticBrush>(customImage)},

                {OldPhoto::getId(), make_shared<OldPhoto>(customImage)},
                {Gama::getId(), make_shared<Gama>(customImage)},
                {Saturation::getId(), make_shared<Saturation>(customImage)},
                {HeatMap::getId(), make_shared<HeatMap>(customImage)},
                {Snow::getId(), make_shared<Snow>(customImage)},
               };

    for (auto &pair : filters) {
        QString name = QString::fromStdString(pair.second->getName());
        QPushButton *btn = new QPushButton(name);
        btn->setMinimumHeight(34);
        scrollLayout->addWidget(btn);

        auto *effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(8);
        effect->setOffset(0);
        effect->setColor(QColor(0, 180, 255, 100));
        btn->setGraphicsEffect(effect);
        btn->installEventFilter(this);

        connect(btn, &QPushButton::clicked, this, [=]() { applyFilter(pair.first, pair.second->getName()); });
    }

    scrollLayout->addStretch();
    scrollContent->setLayout(scrollLayout);
    filtersScroll->setWidget(scrollContent);
    filtersLayout->addWidget(filtersScroll);

    // --------------------------------------------
    // 🎨 منطقة الصورة
    QFrame *canvasFrame = new QFrame;
    canvasFrame->setObjectName("frameCanvas");
    QVBoxLayout *canvasLayout = new QVBoxLayout(canvasFrame);

    imageLabel = new QLabel("Open an image or Drop it Here");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setObjectName("dropZone");
    imageLabel->setMinimumHeight(380);
    canvasLayout->addWidget(imageLabel);
    imageLabel->installEventFilter(this);

    mainLayout->addWidget(filtersFrame);
    mainLayout->addWidget(canvasFrame, 1);

    // ⌨️ اختصارات الكيبورد
    auto addShortcut = [&](QString seq, auto slot) {
        QShortcut *sc = new QShortcut(QKeySequence(seq), this);
        connect(sc, &QShortcut::activated, this, slot);
    };
    addShortcut("Ctrl+O", [=]() { openImage(); });
    addShortcut("Ctrl+S", [=]() { saveImage(); });
    addShortcut("Ctrl+Z", [=]() { undoStackTrigger(); });
    addShortcut("Ctrl+Shift+Z", [=]() { redoStackTrigger(); });
    addShortcut("Ctrl+Y", [=]() { redoStackTrigger(); });
}

shared_ptr<Filter> MainWindow::getFilter(const std::string &name) {
    for (auto &p : filters) {
        if (p.first == name)
            return p.second;
    }
    return nullptr;
}

// --------------------------------------------
void MainWindow::setupToolbar() {
    QToolBar *toolbar = new QToolBar("Toolbar");
    toolbar->setObjectName("mainToolbar");
    toolbar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolbar);

    QAction *openAct = toolbar->addAction("📂 Open");
    QAction *saveAct = toolbar->addAction("💾 Save");
    QAction *undoAct = toolbar->addAction("↩ Undo");
    QAction *redoAct = toolbar->addAction("↪ Redo");

    connect(openAct, &QAction::triggered, this, [=]() { openImage(); });
    connect(saveAct, &QAction::triggered, this, [=]() { saveImage(); });
    connect(undoAct, &QAction::triggered, this, [=]() { undoStackTrigger(); });
    connect(redoAct, &QAction::triggered, this, [=]() { redoStackTrigger(); });

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    QLabel *appTitle = new QLabel("Wake Up at Dawn");
    appTitle->setObjectName("appTitle");
    toolbar->addWidget(appTitle);
}

// --------------------------------------------
void MainWindow::openImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (fileName.isEmpty()) return;
    if (!customImage.loadNewImage(fileName.toStdString())) {
        QMessageBox::warning(this, "Error", "Failed to load image.");
        return;
    }
    currentImage.load(fileName);
    originalImage = customImage;
    imageLabel->setPixmap(currentImage.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    undoStack = std::stack<Image>();
    redoStack = std::stack<Image>();
    statusLabel->setText("✅ Loaded: " + QFileInfo(fileName).fileName());
}

void MainWindow::saveImage() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg *.bmp)");
    if (fileName.isEmpty()) return;
    customImage.saveImage(fileName.toStdString());
    statusLabel->setText("💾 Saved successfully!");
}

void MainWindow::undoStackTrigger() {
    if (!undoStack.empty()) {
        redoStack.push(customImage);
        customImage = undoStack.top();
        undoStack.pop();
        QImage qimg((uchar*)customImage.imageData, customImage.width, customImage.height,
                    customImage.width * 3, QImage::Format_RGB888);
        imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("↩️ Undo");
    }
}

void MainWindow::redoStackTrigger() {
    if (!redoStack.empty()) {
        undoStack.push(customImage);
        customImage = redoStack.top();
        redoStack.pop();
        QImage qimg((uchar*)customImage.imageData, customImage.width, customImage.height,
                    customImage.width * 3, QImage::Format_RGB888);
        imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("↪️ Redo");
    }
}

// --------------------------------------------
void MainWindow::applyFilter(const string &filterId, const string &name, bool skipNeeds) {
    if (customImage.width == 0) {
        QMessageBox::warning(this, "Warning", "Load an image first!");
        return;
    }

    auto filter = getFilter(filterId);
    if (!filter) return;

    if (!skipNeeds) {
        auto needs = filter->getNeeds();
        for (auto &param : needs) {
            bool ok = true;
            if (param.type == "float" || param.type == "int") {
                double val = QInputDialog::getDouble(this, tr("Parameter Needed"),
                                                     QString::fromStdString(param.name),
                                                     QString::fromStdString(param.defaultValue).toDouble(),
                                                     -1000, 1000, 2, &ok);
                if (!ok) return;
                filter->setParam(param.name, val);
            } else if (param.type == "bool") {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this, tr("Toggle Option"), QString::fromStdString(param.name),
                    QMessageBox::Yes | QMessageBox::No);
                filter->setParam(param.name, (reply == QMessageBox::Yes) ? 1 : 0);
            }
        }
    }

    undoStack.push(customImage);
    redoStack = std::stack<Image>();
    filter->apply();

    double wScaleRatio = (double(imageLabel->size().width())/customImage.width);
    double hScaleRatio = (double(imageLabel->size().height())/customImage.height);
    int labelWidth =  wScaleRatio * customImage.width;
    int labelHeight =  hScaleRatio * customImage.height;
    imageLabel->setFixedSize(labelWidth, labelHeight);

    QImage qimg((uchar *)customImage.imageData, customImage.width, customImage.height,
                customImage.width * 3, QImage::Format_RGB888);
    imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
        imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    statusLabel->setText("✅ Applied: " + QString::fromStdString(name));
}

// --------------------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    QPushButton *btn = qobject_cast<QPushButton *>(obj);
    if (btn) {
        auto *effect = qobject_cast<QGraphicsDropShadowEffect *>(btn->graphicsEffect());
        if (effect) {
            if (event->type() == QEvent::Enter) {
                QPropertyAnimation *anim = new QPropertyAnimation(effect, "blurRadius");
                anim->setDuration(200);
                anim->setStartValue(effect->blurRadius());
                anim->setEndValue(20);
                anim->start(QPropertyAnimation::DeleteWhenStopped);
            } else if (event->type() == QEvent::Leave) {
                QPropertyAnimation *anim = new QPropertyAnimation(effect, "blurRadius");
                anim->setDuration(200);
                anim->setStartValue(effect->blurRadius());
                anim->setEndValue(8);
                anim->start(QPropertyAnimation::DeleteWhenStopped);
            }
        }
    }

    // 👇 الضغط المستمر لعرض الصورة الأصلية
    if (obj == imageLabel && customImage.width > 0) {
        if (event->type() == QEvent::MouseButtonPress) {
            QImage qimg((uchar*)originalImage.imageData, originalImage.width, originalImage.height,
                        originalImage.width * 3, QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
                imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            statusLabel->setText("👁️ Showing Original");
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QImage qimg((uchar*)customImage.imageData, customImage.width, customImage.height,
                        customImage.width * 3, QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(
                imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            statusLabel->setText("🎨 Showing Edited");
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// --------------------------------------------
void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QString fileName = mimeData->urls().first().toLocalFile();
        if (fileName.isEmpty()) return;
        if (!customImage.loadNewImage(fileName.toStdString())) {
            QMessageBox::warning(this, "Error", "Failed to load image.");
            return;
        }
        currentImage.load(fileName);
        originalImage = customImage;
        undoStack = std::stack<Image>();
        redoStack = std::stack<Image>();
        imageLabel->setPixmap(currentImage.scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ Loaded via Drag & Drop: " + QFileInfo(fileName).fileName());
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (!currentImage.isNull()) {
        imageLabel->setPixmap(currentImage.scaled(
            imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

MainWindow::~MainWindow() { delete ui; }
