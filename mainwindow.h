#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchButton_clicked();
    void onTableItemClicked(QTableWidgetItem *item);
    void saveDataToFile(const QString &city);
    void loadDataFromFile();
    void onDeleteCityButtonClicked();
    void getWeather(const QString &citys);
    void getRainForecast(const QString &citys);
    void getCurrentLocation();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
