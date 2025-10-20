#pragma once
#include <QMainWindow>
#include <QLabel>
#include <stack>
#include <vector>
#include <memory>
#include "Filters.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void openImage();
    void saveImage();
    void undoStackTrigger();
    void redoStackTrigger();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QLabel *statusLabel;

    Image customImage;
    Image originalImage;
    QPixmap currentImage;
    QString originalImagePath;
    void displayScaledImage(const QImage &img);

    // ✅ Crop logic
    bool croppingMode = false;
    bool cropping = false;
    QPoint cropStart, cropEnd;

    // ✅ Before/After logic
    bool showingOriginal = false;

    // Undo/Redo + Filters
    std::stack<Image> undoStack, redoStack;
    std::vector<std::pair<std::string, std::shared_ptr<Filter>>> filters;

    // 🧩 Methods
    void setupToolbar();
    void applyFilter(const std::string &filterId, const std::string &name, bool skipNeeds = false);
    std::shared_ptr<Filter> getFilter(const std::string &name);
};
