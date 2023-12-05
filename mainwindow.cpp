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


    // Assuming you have a QTableWidget named "tableWidget" in your UI
    ui->tableWidget->setColumnCount(1);  // Set the number of columns

    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "City");
    ui->tableWidget->setColumnWidth(0, 160);
    // Connect the itemClicked signal to a custom slot
    connect(ui->tableWidget, &QTableWidget::itemClicked, this, &MainWindow::onTableItemClicked);
    connect(ui->deleteCityButton, &QPushButton::clicked, this, &MainWindow::onDeleteCityButtonClicked);

    // You may want to load data from a file here
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
        // Add a new row to the table for the entered city
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem(city);

        item->setSizeHint(QSize(200, 30));

        ui->tableWidget->setItem(row, 0, item);

        // Save the data to a file
        saveDataToFile(city);
        ui->cityLineEdit->clear();
        ui->cityLineEdit->setFocus();
    }


}

void MainWindow::onTableItemClicked(QTableWidgetItem *item)
{
    // Handle the click event for the table item
    if (item != nullptr) {
        QString selectedCity = item->text();
        // Do something with the selected city
        qDebug() << "Selected City: " << selectedCity;
        getWeather(selectedCity);
        getRainForecast(selectedCity);


    }
}

// Add this function to your MainWindow class in mainwindow.cpp
void MainWindow::getWeather(const QString &city)
{
    // Replace "YOUR_API_KEY" with the API key you obtained from OpenWeatherMap
    const QString apiKey = "7dc0b0c78fc89d74a233ca477b5b360d";

    // Replace "YOUR_UNITS" with the preferred units (e.g., "metric" for Celsius)
    const QString units = "metric";

    // Construct the URL for the weather API request
    const QString apiUrl = QString("http://api.openweathermap.org/data/2.5/weather?q=%1&units=%2&appid=%3")
                               .arg(city)
                               .arg(units)
                               .arg(apiKey);

     qDebug() << apiUrl;

    // Create a network manager to handle the HTTP request
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

    // Connect the finished signal to a lambda function to handle the response
    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();

            // Extract the temperature information (you can extract more data as needed)
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

            // Update the UI labels (you can adjust this part based on your UI)
            ui->feel_like->setText(QString("Feels %1°C").arg(feelLike.toDouble()));
            ui->max_temp->setText(QString("Max %1°C").arg(maxTemp.toDouble()));
            ui->min_temp->setText(QString("Min %1°C").arg(minTemp.toDouble()));
            ui->temp->setText(QString("Temp %1°C").arg(temp.toDouble()));
            ui->DayType->setText(QString("%1 - %2").arg(description.toString()).arg(location.toString()));

            // Display the weather information (you can update this part based on your UI)
            qDebug() << "Temperature in " << city << ": " << temperature.toDouble() << "°C";

                                                                                       // Clean up
                                                                                       reply->deleteLater();
            networkManager->deleteLater();
        } else {
            // Handle the error
            qDebug() << "Error: " << reply->errorString();

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        }
    });

    // Send the HTTP request
    networkManager->get(QNetworkRequest(QUrl(apiUrl)));
}


// Custom slot to handle item clicks
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



    // QString temperaturePrecipitationStr = QString("Rain %1mm").arg(precipitation[itemIndex].toString());
    // ui->futureRain->setText(temperatureSunsetStr);


    QString temperatureWindStr = QString("Wind %1kmh").arg(windspeedMax[itemIndex].toDouble());
    ui->futureWind->setText(temperatureWindStr);


    QString UVStr = QString("UV %1kmh").arg(UV[itemIndex].toDouble());
    ui->futureUV->setText(UVStr);

    // Implement your logic here based on the clicked item
    qDebug() << "Clicked on item:" << UV[itemIndex];
}


void MainWindow::getForecast(double latitude, double longitude)
{
    // Construct the URL with the query parameters
    QUrl url("https://api.open-meteo.com/v1/forecast");
    QUrlQuery query;
    query.addQueryItem("latitude", QString::number(latitude));
    query.addQueryItem("longitude", QString::number(longitude));
    query.addQueryItem("hourly", "temperature_2m,relativehumidity_2m,apparent_temperature,precipitation,rain,weathercode,surface_pressure,visibility,evapotranspiration,windspeed_10m,winddirection_10m,windgusts_10m,cloudcover,uv_index,dewpoint_2m,precipitation_probability,shortwave_radiation");
    query.addQueryItem("daily", "weathercode,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,precipitation_sum,precipitation_probability_max,windspeed_10m_max,windgusts_10m_max,uv_index_max,rain_sum,winddirection_10m_dominant");
    query.addQueryItem("forecast_days", "15");
    query.addQueryItem("timezone", "auto");
    url.setQuery(query);

    // Create a network request
    QNetworkRequest request(url);

    url.setQuery(query);

    // Send the request
    // manager->get(request);
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);


    // Connect the finished signal to a lambda function to handle the response
    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());

            if (!jsonResponse.isNull() && jsonResponse.isObject())
            {
                // Get the root object
                QJsonObject root = jsonResponse.object();

                // Get the timezone
                QString timezone = root["timezone"].toString();

                // Get the hourly data
                QJsonObject hourly = root["hourly"].toObject();

                QJsonArray times = root["daily"].toObject()["time"].toArray();

                // have a list view name futurePredict add time from the array to list

                // Create a QStringList to store time values
                QStringList timeList;

                // Iterate through the time array and add each time value to the list
                for (const QJsonValue &time : times) {

                    timeList.append(QString("Date: %1").arg(time.toString()));
                }

                // Create a QStringListModel and set it to the QListView
                QStringListModel *model = new QStringListModel(timeList);
                ui->futurePredict->setModel(model);



                // Connect the clicked signal to a lambda function
                connect(ui->futurePredict, &QListView::clicked, [=](const QModelIndex &index) {
                    // Retrieve the clicked item text
                    QString selectedItem = index.data().toString();
                    int itemIndex = index.row();

                    // Pass the selected item to a custom function
                    onListItemClicked(jsonResponse,selectedItem,itemIndex);
                });


                // ui->futurePredict->
                // Get the temperature array
                QJsonArray temperature = hourly["temperature_2m"].toArray();

                // Get the first temperature value
                double temp = temperature[0].toDouble();

                // Print the temperature and timezone
                qDebug() << "The temperature at" << timezone << "is" << temp << "degrees";

            }
        } else {
            // Handle the error
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
    // Replace "YOUR_API_KEY" with the API key you obtained from OpenWeatherMap
    const QString apiKey = "7dc0b0c78fc89d74a233ca477b5b360d";

    // Replace "YOUR_UNITS" with the preferred units (e.g., "metric" for Celsius)
    const QString units = "metric";

    // Construct the URL for the weather API request
    const QString apiUrl = QString("http://api.openweathermap.org/data/2.5/forecast?q=%1&units=%2&appid=%3")
                               .arg(city)
                               .arg(units)
                               .arg(apiKey);

    // Create a network manager to handle the HTTP request
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

    // Connect the finished signal to a lambda function to handle the response
    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response directly
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();

            // Extract the hourly forecast list
            QJsonArray forecastList = jsonObject.value("list").toArray();

            // Loop through the forecast data to find rain information
            for (const QJsonValue &forecast : forecastList) {
                // Extract the timestamp and rain information
                qint64 timestamp = forecast.toObject().value("dt").toInt();
                QJsonObject rainObject = forecast.toObject().value("rain").toObject();

                // Extract rain volume for the next 3 hours (assuming rain is provided in 3h intervals)
                double rainVolume = rainObject.value("3h").toDouble();

                // Check if the timestamp is within the next 2-3 hours
                QDateTime forecastTime = QDateTime::fromSecsSinceEpoch(timestamp);
                QDateTime currentTime = QDateTime::currentDateTime();
                if (forecastTime > currentTime && forecastTime <= currentTime.addSecs(3 * 60 * 60)) {
                    // Display the rain information (you can update this part based on your UI)
                    // qDebug() << "Rain forecast for " << city << " in the next 3 hours: " << rainVolume << " mm";
                    ui->forecastText->setText(QString("Rain forecast for in next 3 hours: %1 mm").arg(rainVolume));
                }
            }

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        } else {
            // Handle the error
            qDebug() << "Error: " << reply->errorString();

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        }
    });


    // Send the HTTP request
    networkManager->get(QNetworkRequest(QUrl(apiUrl)));
}


void MainWindow::saveDataToFile(const QString &city)
{
    // Save the city data to a file
    QFile file("cities.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << city << "\n";
        file.close();
    }
}

void MainWindow::loadDataFromFile()
{
    // Load data from the file and populate the table
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


// Add this slot to your MainWindow class in mainwindow.cpp
void MainWindow::onDeleteCityButtonClicked()
{
    // Get the selected item
    QTableWidgetItem *selectedItem = ui->tableWidget->currentItem();

    if (selectedItem != nullptr) {
        // Get the selected city
        QString selectedCity = selectedItem->text();

        // Find the corresponding row index
        int row = ui->tableWidget->row(selectedItem);

        // Remove the item from the table
        ui->tableWidget->removeRow(row);

        // Update the file without the deleted city
        // updateFileAfterDelete(selectedCity);

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
    // Create a network manager to handle the HTTP request
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);

    // Connect the finished signal to a lambda function to handle the response
    connect(networkManager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response to get the user's location information
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject jsonObject = jsonResponse.object();

            // Extract the location information
            QJsonValue cityValue = jsonObject.value("geo").toObject().value("city");

            // Update the UI label with the user's current location

            // Call the getWeather function with the user's current location
            getWeather(cityValue.toString());

            // Call the getRainForecast function with the user's current location
            getRainForecast(cityValue.toString());

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        } else {
            // Handle the error
            qDebug() << "Error: " << reply->errorString();

            // Clean up
            reply->deleteLater();
            networkManager->deleteLater();
        }
    });

    // Send the HTTP request to get the user's IP location
    networkManager->get(QNetworkRequest(QUrl("https://lumtest.com/myip.json")));
}
