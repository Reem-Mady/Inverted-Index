#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <string>
#include <QVector>
#include <QtGui>
#include<sstream>
#include <iterator>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTableWidget>
#include <dirent.h>

namespace fs = std::experimental::filesystem;
using namespace std;

struct  Node
{
    Node *left;
    Node *right;
    QString word;
    QVector<QString> doc_id;
    Node(QString word, QString doc, Node *left = NULL, Node *right = NULL)
    {
        this->word = word;
        doc_id.push_back(doc);
        this->left = left;
        this->right = right;

    }

    void insert_R(QString word, QString doc)
    {
        right = new Node(word, doc);
    }
    void insert_L(QString word, QString doc)
    {
        left = new Node(word, doc);
    }
    bool is_leaf()
    {
        return (left == NULL && right == NULL);
    }

};
QString To_Lower(QString s)
{
    return s.toLower();
}

QString remove_trash(QString s)
{
    for(int i =0; i<s.length();i++)
    {
        if (s[i] == '?' || s[i] == '.' || s[i] == '!' || s[i] == ',')
            s.remove(i,1);
    }

    return s;
}

bool check_word(QString s)
{
    bool flag;
    for(int i =0; i<s.length(); i++)
    {
        if(s[i].isLetter())
            flag = true;
        else
        {
            flag = false;
            break;
        }
    }
    return flag;
}

class Tree
{
    Node * Tree_root;

public:
    Tree()
    {
        Tree_root=NULL;
    }
    bool insert(QString word,QString doc)
    {
        word =	To_Lower(word);
        word = remove_trash(word);
        bool flag = check_word(word);
        if(!flag)
            return false;
        else
        {
            if (Tree_root == NULL)
            {
                Tree_root = new Node(word, doc);
                return true;
            }
            Node * temp = Tree_root;
            while (temp != NULL)
            {
                QString S_temp = temp->word;

                if (word == S_temp)
                {
                    temp->doc_id.push_back(doc);
                    return false;
                }

                else if (word < S_temp)
                {
                    if (temp->left == NULL)
                    {
                        temp->insert_L(word, doc);
                        return true;
                    }
                    else
                        temp = temp->left;

                }
                else
                {
                    if (temp->right == NULL)
                    {
                        temp->insert_R(word, doc);
                        return true;
                    }
                    else
                        temp = temp->right;
                }

            }
        }

    }

    bool  search(QString word, QVector<QString> & vec)
    {
        word=To_Lower(word);
        if (Tree_root == NULL) //check in main if the vector is empty
        {
            return false;
        }
        Node *temp = Tree_root;
        while (temp != NULL)
        {
            QString S_temp = temp->word;
            if (S_temp == word)
            {
                vec = temp->doc_id;
                return true;
            }

            if (word < S_temp)
            {
                if (temp->left == NULL)
                {
                    return false;
                }
                else
                {
                    temp = temp->left;
                }
            }
            else
            {
                if (word > S_temp)
                {
                    if (temp->right == NULL)
                    {
                        return false;
                    }
                    else
                    {
                        temp = temp->right;
                    }
                }
            }
        }


    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();
    ui->path->setPlaceholderText("Enter the folder path");
    ui->search_word->setPlaceholderText("Enter the word");
}

MainWindow::~MainWindow()
{
    delete ui;
}
string x;
Tree tree;
ifstream file;
QString path1;

void MainWindow::on_Start_clicked()
{    
    ui->progressBar->show();
    //for(int i = 0; i<=100000;i++)
    //{
        QApplication::processEvents();
        /*QFile ifile(ui->path->text()+QString::number(i)+".txt");
        ifile.open(QIODevice::ReadOnly | QIODevice::Text);

        // read whole content
        QString content = ifile.readAll();
        // extract words
        QStringList list = content.split(" ");
        for (int j = 0; j< list.size(); j++)
        {
            if(list[j].endsWith("'s") || list[j].endsWith("'d") || list[j].endsWith("'m") || list[j].endsWith("'re"))
            {
                list[j].truncate(list[j].indexOf("'"));
            }
            tree.insert(list[j],QString::number(i));
        }

        ifile.close();*/
        int i = 0;
        path1 = ui->path->text();
        string path = path1.toLocal8Bit().constData();
        for(auto& p: fs::directory_iterator(path))
        {
            QApplication::processEvents();
            QFile myfile(QString::fromStdString(p.path().string()));
            myfile.open(QIODevice::ReadOnly | QIODevice::Text);
            string s = myfile.fileName().toLocal8Bit().constData();
            int tempo=s.find_last_of("//");
            string na = s.substr(tempo+1);
            QString name = QString::fromStdString(na);
            name.truncate(name.indexOf("."));
            // read whole content
            QString content = myfile.readAll();
            // extract words
            QStringList list = content.split(" ");
            for (int j = 0; j< list.size(); j++)
            {
                if(list[j].endsWith("'s") || list[j].endsWith("'d") || list[j].endsWith("'m") || list[j].endsWith("'re"))
                {
                    list[j].truncate(list[j].indexOf("'"));
                }
                tree.insert(list[j],name);
            }

            myfile.close();
            ui->progressBar->setValue(i/1000);
            i++;
        }

    QMessageBox::information(this,tr("Information"),tr("Done!"));
}

QString search_word;
QVector<QString> docs;
bool av = false;
void MainWindow::on_Search_clicked()
{
    ui->tableWidget->setRowCount(0);

    docs.clear();

    search_word=ui->search_word->text();
    av=tree.search(search_word,docs);
    if(av)
    {
       for(int i=0;i<docs.size();i++)
       {
           ui->tableWidget->insertRow(i);
           ui->tableWidget->setItem(i,0,new QTableWidgetItem(docs[i]));
           QString path2=path1+docs[i]+".txt";
           QFile myfile(path2);
           myfile.open(QIODevice::ReadOnly | QIODevice::Text);
           QString content = myfile.readAll();
           ui->tableWidget->setItem(i,1,new QTableWidgetItem(content));
           ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
           myfile.close();
       }
    }
    else
        QMessageBox::information(this,tr("Error"),tr("Word not found."));
}
