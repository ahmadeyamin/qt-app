#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidgetItem>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariant>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QStringList>
#include <QStringListModel>
#include <QListView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    ui->tableWidget->setColumnCount(1);  

    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "City");
    ui->tableWidget->setColumnWidth(0, 160);
    
    connect(ui->tableWidget, &QTableWidget::itemClicked, this, &MainWindow::onTableItemClicked);
    connect(ui->deleteCityButton, &QPushButton::clicked, this, &MainWindow::onDeleteCityButtonClicked);


    loadDataFromFile();

    getCurrentLocation();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_searchButton_clicked()
{
    QString city = ui->cityLineEdit->text();

    if (city != nullptr) {
        
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem(city);

        item->setSizeHint(QSize(200, 30));

        ui->tableWidget->setItem(row, 0, item);


        saveDataToFile(city);
        ui->cityLineEdit->clear();
        ui->cityLineEdit->setFocus();
    }


}

void MainWindow::onTableItemClicked(QTableWidgetItem *item)
{
    
    if (item != nullptr) {
        QString selectedCity = item->text();
        
        qDebug() << "Selected City: " << selectedCity;
        getWeather(selectedCity);
        getRainForecast(selectedCity);


    }
}

// Add this function to your MainWindow class in mainwindow.cpp
void MainWindow::getWeather(const QString &city)
{
    
    const QString apiKey = "7dc0b0c78fc89d74a233ca477b5b360d";


    const QString units = "metric";

    // Construct the URL for the weather API request
    const QString apiUrl = QString("http://api.openweathermap.org/data/2.5/weather?q=%1&units=%2&appid=%3")
                               .arg(city)
                               .arg(units)
                               .arg(apiKey);

     qDebug() << apiUrl;


    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);


    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();


            QJsonValue main = jsonObject.value("main");
            QJsonValue lonlet = jsonObject.value("coord");
            QJsonValue temperature = main.toObject().value("temp");

            getForecast(lonlet.toObject().value("let").toDouble(),lonlet.toObject().value("lon").toDouble());

            QJsonValue weather = jsonObject.value("weather").toArray().at(0).toObject();
            QJsonValue feelLike = main.toObject().value("feels_like");
            QJsonValue location = jsonObject.value("name");
            QJsonValue maxTemp = main.toObject().value("temp_max");
            QJsonValue minTemp = main.toObject().value("temp_min");
            QJsonValue temp = main.toObject().value("temp");
            QJsonValue description = weather.toObject().value("main");


            ui->feel_like->setText(QString("Feels %1°C").arg(feelLike.toDouble()));
            ui->max_temp->setText(QString("Max %1°C").arg(maxTemp.toDouble()));
            ui->min_temp->setText(QString("Min %1°C").arg(minTemp.toDouble()));
            ui->temp->setText(QString("Temp %1°C").arg(temp.toDouble()));
            ui->DayType->setText(QString("%1 - %2").arg(description.toString()).arg(location.toString()));


            qDebug() << "Temperature in " << city << ": " << temperature.toDouble() << "°C";

                                                                                       // Clean up
                                                                                       reply->deleteLater();
            networkManager->deleteLater();
        } else {
            
            qDebug() << "Error: " << reply->errorString();


            reply->deleteLater();
            networkManager->deleteLater();
        }
    });


    networkManager->get(QNetworkRequest(QUrl(apiUrl)));
}



void MainWindow::onListItemClicked(const QJsonDocument &jsonData, const QString &selectedItem, int itemIndex)
{


    QJsonArray times = jsonData["daily"].toObject()["time"].toArray();
    QJsonArray temperatureMax = jsonData["daily"].toObject()["temperature_2m_max"].toArray();
    QJsonArray temperatureMin = jsonData["daily"].toObject()["temperature_2m_min"].toArray();
    QJsonArray sunrise = jsonData["daily"].toObject()["sunrise"].toArray();
    QJsonArray sunset = jsonData["daily"].toObject()["sunset"].toArray();
    QJsonArray precipitation = jsonData["daily"].toObject()["precipitation"].toArray();

    QJsonArray windspeedMax = jsonData["daily"].toObject()["windspeed_10m_max"].toArray();
    QJsonArray UV = jsonData["daily"].toObject()["uv_index_max"].toArray();

    QString temperatureMaxStr = QString("Max %1°C").arg(temperatureMax[itemIndex].toDouble());
    ui->futureMax->setText(temperatureMaxStr);

    QString temperatureMinStr = QString("Min %1°C").arg(temperatureMin[itemIndex].toDouble());
    ui->futureMin->setText(temperatureMinStr);


    QString temperatureSunriseStr = QString("Sunrise %1").arg(sunrise[itemIndex].toString());
    ui->futureRise->setText(temperatureSunriseStr);


    QString temperatureSunsetStr = QString("Sunset %1").arg(sunset[itemIndex].toString());
    ui->futureSet->setText(temperatureSunsetStr);





    QString temperatureWindStr = QString("Wind %1kmh").arg(windspeedMax[itemIndex].toDouble());
    ui->futureWind->setText(temperatureWindStr);


    QString UVStr = QString("UV %1kmh").arg(UV[itemIndex].toDouble());
    ui->futureUV->setText(UVStr);
    
    qDebug() << "Clicked on item:" << UV[itemIndex];
}


void MainWindow::getForecast(double latitude, double longitude)
{
    
    QUrl url("https://api.open-meteo.com/v1/forecast");
    QUrlQuery query;
    query.addQueryItem("latitude", QString::number(latitude));
    query.addQueryItem("longitude", QString::number(longitude));
    query.addQueryItem("hourly", "temperature_2m,relativehumidity_2m,apparent_temperature,precipitation,rain,weathercode,surface_pressure,visibility,evapotranspiration,windspeed_10m,winddirection_10m,windgusts_10m,cloudcover,uv_index,dewpoint_2m,precipitation_probability,shortwave_radiation");
    query.addQueryItem("daily", "weathercode,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,precipitation_sum,precipitation_probability_max,windspeed_10m_max,windgusts_10m_max,uv_index_max,rain_sum,winddirection_10m_dominant");
    query.addQueryItem("forecast_days", "15");
    query.addQueryItem("timezone", "auto");
    url.setQuery(query);


    QNetworkRequest request(url);

    url.setQuery(query);


    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);



    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
   
   
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());

            if (!jsonResponse.isNull() && jsonResponse.isObject())
            {
                
                QJsonObject root = jsonResponse.object();

                QString timezone = root["timezone"].toString();


                QJsonObject hourly = root["hourly"].toObject();

                QJsonArray times = root["daily"].toObject()["time"].toArray();


                QStringList timeList;


                for (const QJsonValue &time : times) {

                    timeList.append(QString("Date: %1").arg(time.toString()));
                }


                QStringListModel *model = new QStringListModel(timeList);
                ui->futurePredict->setModel(model);




                connect(ui->futurePredict, &QListView::clicked, [=](const QModelIndex &index) {


                    QString selectedItem = index.data().toString();
                    int itemIndex = index.row();


                    onListItemClicked(jsonResponse,selectedItem,itemIndex);
                });

                QJsonArray temperature = hourly["temperature_2m"].toArray();


                double temp = temperature[0].toDouble();

                qDebug() << "The temperature at" << timezone << "is" << temp << "degrees";

            }
        } else {
            qDebug() << "Error: " << reply->errorString();

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        }
    });


    // Send the HTTP request
    networkManager->get(QNetworkRequest(QUrl(url)));
}



void MainWindow::getRainForecast(const QString &city)
{
    
    const QString apiKey = "7dc0b0c78fc89d74a233ca477b5b360d";


    const QString units = "metric";

    // Construct the URL for the weather API request
    const QString apiUrl = QString("http://api.openweathermap.org/data/2.5/forecast?q=%1&units=%2&appid=%3")
                               .arg(city)
                               .arg(units)
                               .arg(apiKey);


    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);


    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();


            QJsonArray forecastList = jsonObject.value("list").toArray();


            for (const QJsonValue &forecast : forecastList) {
                
                qint64 timestamp = forecast.toObject().value("dt").toInt();
                QJsonObject rainObject = forecast.toObject().value("rain").toObject();


                double rainVolume = rainObject.value("3h").toDouble();


                QDateTime forecastTime = QDateTime::fromSecsSinceEpoch(timestamp);
                QDateTime currentTime = QDateTime::currentDateTime();
                if (forecastTime > currentTime && forecastTime <= currentTime.addSecs(3 * 60 * 60)) {
                    
                    ui->forecastText->setText(QString("Rain forecast for in next 3 hours: %1 mm").arg(rainVolume));
                }
            }

            reply->deleteLater();
            networkManager->deleteLater();
        } else {
            qDebug() << "Error: " << reply->errorString();

            reply->deleteLater();
            networkManager->deleteLater();
        }
    });


    networkManager->get(QNetworkRequest(QUrl(apiUrl)));
}


void MainWindow::saveDataToFile(const QString &city)
{
    QFile file("cities.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << city << "\n";
        file.close();
    }
}

void MainWindow::loadDataFromFile()
{
    QFile file("cities.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString city = stream.readLine();
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            QTableWidgetItem *item = new QTableWidgetItem(city);
            ui->tableWidget->setItem(row, 0, item);
        }
        file.close();
    }
}


void MainWindow::onDeleteCityButtonClicked()
{
    // Get the selected item
    QTableWidgetItem *selectedItem = ui->tableWidget->currentItem();

    if (selectedItem != nullptr) {
        // Get the selected city
        QString selectedCity = selectedItem->text();

        int row = ui->tableWidget->row(selectedItem);

        ui->tableWidget->removeRow(row);


        QStringList cities;
        QFile file("cities.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QString city = stream.readLine();
                cities.append(city);
            }
            file.close();
        }

        // Remove the deleted city from the list
        cities.removeOne(selectedCity);

        // Write the updated data back to the file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            for (const QString &city : cities) {
                stream << city << "\n";
            }
            file.close();
        }
    }
}

void MainWindow::getCurrentLocation()
{
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();

            QJsonValue cityValue = jsonObject.value("geo").toObject().value("city");


            getWeather(cityValue.toString());

            getRainForecast(cityValue.toString());

            reply->deleteLater();
            networkManager->deleteLater();
        } else {
            qDebug() << "Error: " << reply->errorString();

            reply->deleteLater();
            networkManager->deleteLater();
        }
    });

    networkManager->get(QNetworkRequest(QUrl("https://lumtest.com/myip.json")));
}
