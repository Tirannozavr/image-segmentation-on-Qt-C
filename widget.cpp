#include <QColor>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QRgb>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>

#include "ui_widget.h"
#include "widget.h"

QString str;
QString sigmaSTR;
QImage image;

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
  ui->setupUi(this);
}

Widget::~Widget() { delete ui; }

void Widget::on_pushButton_clicked() {
  QImage im;

  //считываем путь к файлу
  str = ui->lineEdit->text();
  str.replace("\\", "\\\\");
  bool op = image.load(str);

  if (op == false) {
    ui->label->setText("file not found");
  } else {
    int i = 1;
    
    int **mas = new int *[image.height()];
    for (int j = 0; j < image.height(); j++) {
      mas[j] = new int[image.width()];
    }
    for (int j = 0; j < image.height(); j++) {
      for (int z = 0; z < image.width(); z++) {
        mas[j][z] = 0;
      }
    }

    //считываем сигму
    sigmaSTR = ui->lineEdit_2->text();
    double sig = (sigmaSTR.toDouble());
    vector<int> vecC;
    vecC.push_back(0);

    vector<double> vecI;
    vecI.push_back(0);

    for (int j = 0; j < image.height(); j++) {
      for (int z = 0; z < image.width(); z++) {
        double iA = image.pixelColor(z, j).valueF();
        double iB = 0, iC = 0;
        if (z != 0 && vecC[(mas[j][z - 1])] != 0) {
          iB = vecI[(mas[j][z - 1])] / vecC[(mas[j][z - 1])];
        }
        if (j != 0 && vecC[(mas[j - 1][z])] != 0) {
          iC = vecI[(mas[j - 1][z])] / vecC[(mas[j - 1][z])];
        }
        if (j == 0) {
          if (z == 0 || (z != 0 && abs(iA - vecI[(mas[j][z - 1])] /
                                                vecC[mas[j][z - 1]]) > sig)) {
            vecC.push_back(1);
            vecI.push_back(iA);
            mas[j][z] = i;
            i++;
          } else {
            mas[j][z] = mas[j][z - 1];
            vecC[mas[j][z]] += 1;
            vecI[mas[j][z]] += iA;
          }
        } else if (z == 0) {
          if (abs(iA - vecI[(mas[j - 1][z])] / vecC[mas[j - 1][z]]) > sig) {
            vecC.push_back(1);
            vecI.push_back(iA);
            mas[j][z] = i;
            i++;
          } else {
            mas[j][z] = mas[j - 1][z];
            vecC[mas[j][z]] += 1;
            vecI[mas[j][z]] += iA;
          }
        } else if (abs(iA - iB) > sig && abs(iA - iC) > sig) {
          vecC.push_back(1);
          vecI.push_back(iA);
          mas[j][z] = i;
          i++;
        } else if ((abs(iA - iB) <= sig) ^ (abs(iA - iC) <= sig)) {
          vecC.push_back(1);
          vecI.push_back(iA);
          mas[j][z] = i;
          i++;
        } else {
          if (abs(iB - iC) <= sig) {  //всегда считаем по верхнему региону
            if (mas[j - 1][z] != mas[j][z - 1]) {
              if (z != 0 && j != 0) {
                vecC[(mas[j - 1][z])] =
                    vecC[(mas[j - 1][z])] + vecC[(mas[j][z - 1])] + 1;
                vecI[(mas[j - 1][z])] =
                    vecI[(mas[j - 1][z])] + vecI[(mas[j][z - 1])] + iA;

                int p = mas[j - 1][z];
                mas[j][z] = p;
                int p1 = mas[j][z - 1];

                for (int i1 = 0; i1 <= j; i1++) {
                  for (int j1 = 0; j1 < image.width(); j1++) {
                    if (mas[i1][j1] == p1) {
                      mas[i1][j1] = p;
                    }
                  }
                }
              }
            } else {
              vecC[(mas[j - 1][z])] += 1;
              vecI[(mas[j - 1][z])] += iA;
              mas[j][z] = mas[j - 1][z];
            }
          } else {
            if (abs(iA - iB) < abs(iA - iC)) {
              if (z != 0) {
                vecC[(mas[j][z - 1])] += 1;
                vecI[(mas[j][z - 1])] += iA;
                mas[j][z] = mas[j][z - 1];
              }
            } else {
              if (j != 0) {
                vecC[(mas[j - 1][z])] += 1;
                vecI[(mas[j - 1][z])] += iA;
                mas[j][z] = mas[j - 1][z];
              }
            }
          }
        }
      }
    }

    int R = 0, G = 0, B = 0;

    for (int n = 0; n < image.height(); n++) {
      for (int m = 0; m < image.width(); m++) {
        QColor col;
        int c = mas[n][m] * 100;
        R = c >> 16;
        G = (c - (R << 16)) >> 8;
        B = (c - (R << 16) - (G << 8));

        R += 20;
        if (R > 255) {
          while (R > 255) {
            R -= 255;
          }
        }
        if (G > 255) {
          while (G > 255) {
            G -= 255;
          }
        }
        if (B > 255) {
          while (B > 255) {
            B -= 255;
          }
        }

        col.setRed(R);
        col.setGreen(G);
        col.setBlue(B);
        image.setPixelColor(m, n, col);
      }
    }

    ui->label->setScaledContents(true);
    ui->label->setPixmap(QPixmap::fromImage(image));
  }
}

void Widget::on_pushButton_2_clicked() {
  QString sigma = sigmaSTR;
  sigma.replace(".", "_");

  int i = str.indexOf(".");
  QString s = str;

  QString newFile = s.insert(i, '_' + sigma);
  image.save(newFile);
  ui->label->setText("image saved successfully");
}
