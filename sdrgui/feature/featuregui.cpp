///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2020 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QCloseEvent>
#include <QStyle>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizeGrip>
#include <QTextEdit>
#include <QObjectCleanupHandler>
#include <QDesktopServices>

#include "mainwindow.h"
#include "gui/workspaceselectiondialog.h"
#include "featuregui.h"

FeatureGUI::FeatureGUI(QWidget *parent) :
    QMdiSubWindow(parent),
    m_featureIndex(0),
    m_contextMenuType(ContextMenuNone),
    m_drag(false),
    m_resizer(this)
{
    qDebug("FeatureGUI::FeatureGUI");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setObjectName("FeatureGUI");
    setStyleSheet(QString(tr("#FeatureGUI { border: 1px solid %1; background-color: %2; }")
        .arg(palette().highlight().color().darker(115).name()))
        .arg(palette().dark().color().darker(115).name()));

    m_indexLabel = new QLabel();
    m_indexLabel->setFixedSize(40, 16);
    m_indexLabel->setStyleSheet("QLabel { background-color: rgb(128, 128, 128); qproperty-alignment: AlignCenter; }");
    m_indexLabel->setText(tr("F:%1").arg(m_featureIndex));
    m_indexLabel->setToolTip("Feature index");

    m_settingsButton = new QPushButton();
    m_settingsButton->setFixedSize(20, 20);
    QIcon settingsIcon(":/gear.png");
    m_settingsButton->setIcon(settingsIcon);
    m_settingsButton->setToolTip("Common settings");

    m_titleLabel = new QLabel();
    m_titleLabel->setText("Feature");
    m_titleLabel->setToolTip("Feature name");
    m_titleLabel->setFixedHeight(20);
    m_titleLabel->setMinimumWidth(20);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    m_helpButton = new QPushButton();
    m_helpButton->setFixedSize(20, 20);
    QIcon helpIcon(":/help.png");
    m_helpButton->setIcon(helpIcon);
    m_helpButton->setToolTip("Show feature documentation in browser");

    m_moveButton = new QPushButton();
    m_moveButton->setFixedSize(20, 20);
    QIcon moveIcon(":/exit.png");
    m_moveButton->setIcon(moveIcon);
    m_moveButton->setToolTip("Move to another workspace");

    m_shrinkButton = new QPushButton();
    m_shrinkButton->setFixedSize(20, 20);
    QIcon shrinkIcon(":/shrink.png");
    m_shrinkButton->setIcon(shrinkIcon);
    m_shrinkButton->setToolTip("Adjust window to minimum size");

    m_closeButton = new QPushButton();
    m_closeButton->setFixedSize(20, 20);
    QIcon closeIcon(":/cross.png");
    m_closeButton->setIcon(closeIcon);
    m_closeButton->setToolTip("Close feature");

    m_statusLabel = new QLabel();
    // m_statusLabel->setText("OK"); // for future use
    m_statusLabel->setFixedHeight(20);
    m_statusLabel->setMinimumWidth(20);
    m_statusLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    m_statusLabel->setToolTip("Feature status");

    m_layouts = new QVBoxLayout();
    m_layouts->setContentsMargins(m_resizer.m_gripSize, m_resizer.m_gripSize, m_resizer.m_gripSize, m_resizer.m_gripSize);
    m_layouts->setSpacing(0);

    m_topLayout = new QHBoxLayout();
    m_topLayout->setContentsMargins(0, 0, 0, 0);
    m_topLayout->addWidget(m_indexLabel);
    m_topLayout->addWidget(m_settingsButton);
    m_topLayout->addWidget(m_titleLabel);
    // m_topLayout->addStretch(1);
    m_topLayout->addWidget(m_helpButton);
    m_topLayout->addWidget(m_moveButton);
    m_topLayout->addWidget(m_shrinkButton);
    m_topLayout->addWidget(m_closeButton);

    m_centerLayout = new QHBoxLayout();
    m_centerLayout->addWidget(&m_rollupContents);

    m_bottomLayout = new QHBoxLayout();
    m_bottomLayout->setContentsMargins(0, 0, 0, 0);
    m_bottomLayout->addWidget(m_statusLabel);
    m_sizeGripBottomRight = new QSizeGrip(this);
    m_sizeGripBottomRight->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_sizeGripBottomRight->setFixedHeight(20);
    // m_bottomLayout->addStretch(1);
    m_bottomLayout->addWidget(m_sizeGripBottomRight, 0, Qt::AlignBottom | Qt::AlignRight);

    m_layouts->addLayout(m_topLayout);
    m_layouts->addLayout(m_centerLayout);
    m_layouts->addLayout(m_bottomLayout);

    QObjectCleanupHandler().add(layout());
    setLayout(m_layouts);

    connect(m_settingsButton, SIGNAL(clicked()), this, SLOT(activateSettingsDialog()));
    connect(m_helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
    connect(m_moveButton, SIGNAL(clicked()), this, SLOT(openMoveToWorkspaceDialog()));
    connect(m_shrinkButton, SIGNAL(clicked()), this, SLOT(shrinkWindow()));
    connect(this, SIGNAL(forceShrink()), this, SLOT(shrinkWindow()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(
        &m_rollupContents,
        &RollupContents::widgetRolled,
        this,
        &FeatureGUI::onWidgetRolled
    );

    m_resizer.enableChildMouseTracking();
}

FeatureGUI::~FeatureGUI()
{
    qDebug("FeatureGUI::~FeatureGUI");
    delete m_sizeGripBottomRight;
    delete m_bottomLayout;
    delete m_centerLayout;
    delete m_topLayout;
    delete m_layouts;
    delete m_statusLabel;
    delete m_closeButton;
    delete m_shrinkButton;
    delete m_moveButton;
    delete m_helpButton;
    delete m_titleLabel;
    delete m_settingsButton;
    delete m_indexLabel;
    qDebug("FeatureGUI::~FeatureGUI: end");
}

void FeatureGUI::closeEvent(QCloseEvent *event)
{
    qDebug("FeatureGUI::closeEvent");
    emit closing();
    event->accept();
}

void FeatureGUI::mousePressEvent(QMouseEvent* event)
{
    if ((event->button() == Qt::LeftButton) && isOnMovingPad())
    {
        m_drag = true;
        m_DragPosition = event->globalPos() - pos();
        event->accept();
    }
    else
    {
        m_resizer.mousePressEvent(event);
    }
}

void FeatureGUI::mouseReleaseEvent(QMouseEvent* event)
{
    m_resizer.mouseReleaseEvent(event);
}

void FeatureGUI::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && isOnMovingPad())
    {
        move(event->globalPos() - m_DragPosition);
        event->accept();
    }
    else
    {
        m_resizer.mouseMoveEvent(event);
    }
}

void FeatureGUI::leaveEvent(QEvent* event)
{
    m_resizer.leaveEvent(event);
    QMdiSubWindow::leaveEvent(event);
}

void FeatureGUI::activateSettingsDialog()
{
    QPoint p = QCursor::pos();
    m_contextMenuType = ContextMenuChannelSettings;
    emit customContextMenuRequested(p);
}

void FeatureGUI::showHelp()
{
    if (m_helpURL.isEmpty()) {
        return;
    }

    QString url;

    if (m_helpURL.startsWith("http")) {
        url = m_helpURL;
    } else {
        url = QString("https://github.com/f4exb/sdrangel/blob/master/%1").arg(m_helpURL); // Something like "plugins/channelrx/chanalyzer/readme.md"
    }

    QDesktopServices::openUrl(QUrl(url));
}

void FeatureGUI::openMoveToWorkspaceDialog()
{
    int numberOfWorkspaces = MainWindow::getInstance()->getNumberOfWorkspaces();
    WorkspaceSelectionDialog dialog(numberOfWorkspaces, this);
    dialog.exec();

    if (dialog.hasChanged()) {
        emit moveToWorkspace(dialog.getSelectedIndex());
    }
}

void FeatureGUI::onWidgetRolled(QWidget *widget, bool show)
{
    if (show)
    {
        // qDebug("FeatureGUI::onWidgetRolled: show: %d %d", m_rollupContents.height(), widget->height());
        int dh = m_heightsMap.contains(widget) ? m_heightsMap[widget] - widget->height() : widget->minimumHeight();
        resize(width(), 52 +  m_rollupContents.height() + dh);
    }
    else
    {
        // qDebug("FeatureGUI::onWidgetRolled: hide: %d %d", m_rollupContents.height(), widget->height());
        m_heightsMap[widget] = widget->height();
        resize(width(), 52 +  m_rollupContents.height());
    }
}

void FeatureGUI::shrinkWindow()
{
    qDebug("FeatureGUI::shrinkWindow");
    adjustSize();
    resize(width(), m_rollupContents.height() + getAdditionalHeight());
}

void FeatureGUI::setTitle(const QString& title)
{
    m_titleLabel->setText(title);
}

bool FeatureGUI::isOnMovingPad()
{
    return m_indexLabel->underMouse() || m_titleLabel->underMouse() || m_statusLabel->underMouse();
}

void FeatureGUI::setIndex(int index)
{
    m_featureIndex = index;
    m_indexLabel->setText(tr("F:%1").arg(m_featureIndex));
}

void FeatureGUI::setDisplayedame(const QString& name)
{
    m_displayedName = name;
    m_indexLabel->setToolTip(tr("%1").arg(m_displayedName));
}
