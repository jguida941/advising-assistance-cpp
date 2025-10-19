#include "mainwindow.hpp"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>

// Constructs the advisor dashboard window and wires up the shared catalog.
MainWindow::MainWindow(Catalog catalogToUse, QWidget* parent)
    : QMainWindow(parent),
      catalog(std::move(catalogToUse)) {
    createMenus();
    createLayout();

    searchDelayTimer = new QTimer(this);  // Kick off search after the user pauses typing.
    searchDelayTimer->setSingleShot(true);
    searchDelayTimer->setInterval(300);
    connect(searchDelayTimer, &QTimer::timeout, this, &MainWindow::performSearch);

    refreshCourseList();
    statusBar()->showMessage("Ready");  // Match the CLI startup message tone.
}

MainWindow::~MainWindow() = default;

// Builds the File and View menus that drive the dashboard actions.
void MainWindow::createMenus() {
    auto* fileMenu = menuBar()->addMenu(tr("File"));
    auto* openAction = fileMenu->addAction(tr("Open CSV…"));
    auto* reloadAction = fileMenu->addAction(tr("Reload"));
    fileMenu->addSeparator();
    auto* exitAction = fileMenu->addAction(tr("Exit"));

    connect(openAction, &QAction::triggered, this, &MainWindow::openCatalog);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadCatalog);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto* viewMenu = menuBar()->addMenu(tr("View"));
    auto* missingAction = viewMenu->addAction(tr("Show Missing Prereqs"));
    connect(missingAction, &QAction::triggered, this, &MainWindow::showMissingPrerequisites);
}

// Lays out the search tools, course list, details pane, and warning list.
void MainWindow::createLayout() {
    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    // Top row keeps the quick search field easy to reach.
    auto* searchLayout = new QHBoxLayout();
    auto* searchLabel = new QLabel(tr("Find course:"), centralWidget);
    searchField = new QLineEdit(centralWidget);
    searchField->setPlaceholderText(tr("e.g., CSCI200"));
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchField);
    rootLayout->addLayout(searchLayout);

    // Splitter holds the course list on the left and details on the right.
    auto* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    courseListModel = new CourseListModel(splitter);
    courseListView = new QListView(splitter);
    courseListView->setModel(courseListModel);
    courseListView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* detailWidget = new QWidget(splitter);
    auto* detailLayout = new QVBoxLayout(detailWidget);
    detailLayout->setContentsMargins(8, 0, 0, 0);
    detailLayout->setSpacing(8);

    courseTitleLabel = new QLabel(tr("Select a course to view details"), detailWidget);  // Placeholder until something loads.
    courseTitleLabel->setWordWrap(true);
    courseTitleLabel->setProperty("heading", true);
    detailLayout->addWidget(courseTitleLabel);

    auto* prereqTitle = new QLabel(tr("Prerequisites"), detailWidget);
    detailLayout->addWidget(prereqTitle);

    prerequisiteList = new QListWidget(detailWidget);  // Mirrors the CLI prerequisite printout.
    prerequisiteList->setSelectionMode(QAbstractItemView::SingleSelection);
    prerequisiteList->setUniformItemSizes(true);
    detailLayout->addWidget(prerequisiteList, 1);

    warningsTitleLabel = new QLabel(tr("Catalog Warnings"), detailWidget);
    warningsTitleLabel->setVisible(false);
    detailLayout->addWidget(warningsTitleLabel);

    warningsList = new QListWidget(detailWidget);  // Shows loader warnings without blocking the UI.
    warningsList->setSelectionMode(QAbstractItemView::NoSelection);
    warningsList->setFocusPolicy(Qt::NoFocus);
    warningsList->setVisible(false);
    detailLayout->addWidget(warningsList);

    splitter->addWidget(courseListView);
    splitter->addWidget(detailWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    rootLayout->addWidget(splitter, 1);

    setCentralWidget(centralWidget);

    connect(courseListView, &QListView::clicked, this, &MainWindow::handleCourseSelection);
    connect(prerequisiteList, &QListWidget::itemActivated, this, &MainWindow::handlePrerequisiteActivated);
    connect(prerequisiteList, &QListWidget::itemDoubleClicked, this, &MainWindow::handlePrerequisiteActivated);
    connect(searchField, &QLineEdit::textChanged, this, &MainWindow::handleSearchEdited);
}

// Prompts the user for a CSV catalog file and loads it when chosen.
void MainWindow::openCatalog() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Catalog"),
        currentCatalogPath,
        tr("CSV Files (*.csv);;All Files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    loadCatalogFromPath(filePath);  // Let the shared loader handle validation and caching.
}

// Reloads the most recently opened catalog so edits can be picked up quickly.
void MainWindow::reloadCatalog() {
    if (currentCatalogPath.isEmpty()) {
        QMessageBox::information(this, tr("Reload Catalog"), tr("Load a catalog first."));
        return;
    }
    loadCatalogFromPath(currentCatalogPath);  // Re-run the core loader using the stored path.
}

// Updates the detail pane when a course is selected from the list view.
void MainWindow::handleCourseSelection(const QModelIndex& index) {
    const QString courseId = courseListModel->courseIdForRow(index.row());
    if (courseId.isEmpty()) {
        return;
    }
    const Course* course = catalog.get(courseId.toStdString());  // Pull the full course record from the core.
    populateCourseDetails(course);  // Show the course details in the right pane.
}

// Starts a short timer so course searches only fire after the user pauses typing.
void MainWindow::handleSearchEdited(const QString& text) {
    if (text.trimmed().isEmpty()) {
        searchDelayTimer->stop();
        return;
    }
    searchDelayTimer->start();
}

// Looks up the typed course ID, highlights it in the list, and shows its details.
void MainWindow::performSearch() {
    const QString trimmed = searchField->text().trimmed().toUpper();
    if (trimmed.isEmpty()) {
        return;
    }

    const Course* course = catalog.get(trimmed.toStdString());
    if (!course) {
        statusBar()->showMessage(tr("Course not found: %1").arg(trimmed), 4000);  // Same wording as the CLI error.
        return;
    }

    populateCourseDetails(course);  // Reuse the same detail builder used by list selection.

    const std::vector<std::string> ids = catalog.ids();
    for (int row = 0; row < static_cast<int>(ids.size()); ++row) {
        if (QString::fromStdString(ids[row]).compare(trimmed, Qt::CaseInsensitive) == 0) {
            // Keep the selection synced with the search box like the CLI lookup.
            const QModelIndex targetIndex = courseListModel->index(row, 0);
            courseListView->selectionModel()->select(
                targetIndex,
                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            courseListView->scrollTo(targetIndex, QListView::PositionAtCenter);
            break;
        }
    }
}

// Double-clicking or pressing enter on a prerequisite jumps directly to that course.
void MainWindow::handlePrerequisiteActivated(QListWidgetItem* item) {
    if (!item) {
        return;
    }
    const QString courseId = item->data(Qt::UserRole).toString();
    if (courseId.isEmpty()) {
        return;
    }
    searchField->setText(courseId);  // Mirror the ID into the search box for clarity.
    performSearch();
}

// Displays the missing prerequisite list captured during the last catalog load.
void MainWindow::showMissingPrerequisites() {
    if (lastLoadResult.missingPrerequisites.empty()) {
        QMessageBox::information(this, tr("Missing Prerequisites"), tr("All prerequisites were found in the catalog."));  // Stay consistent with CLI success message.
        return;
    }

    QStringList lines;
    for (const auto& missing : lastLoadResult.missingPrerequisites) {
        lines << QString::fromStdString(missing);
    }

    QMessageBox::information(
        this,
        tr("Missing Prerequisites"),
        tr("The following prerequisites reference missing courses:\n\n%1").arg(lines.join('\n')));
}

// Loads the catalog from disk, refreshes the models, and surfaces any warnings.
void MainWindow::loadCatalogFromPath(const QString& path) {
    LoadResult result = catalog.load(path.toStdString());
    lastLoadResult = result;

    if (!result.ok) {
        updateWarningsPane(result);  // Still show any warnings so the user knows what went wrong.
        statusBar()->showMessage(tr("Unable to load catalog: %1").arg(path), 4000);
        return;
    }

    currentCatalogPath = QString::fromStdString(result.path);
    refreshCourseList();  // Pull in the new IDs for the list view.
    updateStatusFromLoad(result);
    updateWarningsPane(result);  // Keep the warning panel in sync with the latest messages.
}

// Fills the detail pane with the selected course and its prerequisite status.
void MainWindow::populateCourseDetails(const Course* course) {
    if (!course) {
        courseTitleLabel->setText(tr("Course not found."));
        prerequisiteList->clear();
        return;
    }

    courseTitleLabel->setText(
        tr("%1 — %2").arg(QString::fromStdString(course->courseNumber),
                          QString::fromStdString(course->courseName)));

    prerequisiteList->clear();
    if (course->prerequisites.empty()) {
        auto* item = new QListWidgetItem(tr("Prerequisites: none"));  // Match the CLI wording.
        prerequisiteList->addItem(item);
        return;
    }

    for (const auto& prereqId : course->prerequisites) {
        const Course* prereqCourse = catalog.get(prereqId);
        auto* item = new QListWidgetItem(QString::fromStdString(prereqId));
        item->setData(Qt::UserRole, QString::fromStdString(prereqId));

        if (prereqCourse) {
            item->setToolTip(QString::fromStdString(prereqCourse->courseName));
            item->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        } else {
            item->setToolTip(tr("Missing from catalog"));  // Same warning the console path prints.
            item->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        }
        prerequisiteList->addItem(item);
    }
}

// Pushes the latest sorted course IDs into the list model backing the view.
void MainWindow::refreshCourseList() {
    courseListModel->setCourseIds(catalog.ids());
}

// Announces the latest load result in the status bar so the user knows what happened.
void MainWindow::updateStatusFromLoad(const LoadResult& result) {
    statusBar()->showMessage(
        tr("Loaded %1 courses from %2").arg(result.courses).arg(QString::fromStdString(result.path)));
}

// Shows or hides the warning pane based on the messages returned from the core loader.
void MainWindow::updateWarningsPane(const LoadResult& result) {
    warningsList->clear();

    const bool hasWarnings = !result.warnings.empty();  // Only show the pane when there is content.
    warningsTitleLabel->setVisible(hasWarnings);
    warningsList->setVisible(hasWarnings);

    if (!hasWarnings) {
        return;
    }

    for (const auto& warning : result.warnings) {
        warningsList->addItem(QString::fromStdString(warning));  // Mirror the CLI warning text in the panel.
    }
}
