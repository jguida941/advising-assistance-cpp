#pragma once

#include "catalog.hpp"
#include "models.hpp"

#include <QMainWindow>
#include <QModelIndex>
#include <QString>

class QLabel;
class QLineEdit;
class QListView;
class QListWidget;
class QListWidgetItem;
class QStatusBar;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Catalog catalog, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openCatalog();
    void reloadCatalog();
    void handleCourseSelection(const QModelIndex& index);
    void handleSearchEdited(const QString& text);
    void performSearch();
    void handlePrerequisiteActivated(QListWidgetItem* item);
    void showMissingPrerequisites();

private:
    void createMenus();
    void createLayout();
    void loadCatalogFromPath(const QString& path);
    void populateCourseDetails(const Course* course);
    void refreshCourseList();
    void updateStatusFromLoad(const LoadResult& result);
    void updateWarningsPane(const LoadResult& result);

    Catalog catalog;                 // Shared core used by both CLI and GUI paths.
    LoadResult lastLoadResult;       // Remember the latest load outcome for warnings.
    QString currentCatalogPath;      // Stores the last opened file so reload works.

    CourseListModel* courseListModel = nullptr;  // Left-hand course ID list model.
    QListView* courseListView = nullptr;         // List view showing the IDs.
    QLineEdit* searchField = nullptr;            // Quick search input field.
    QLabel* courseTitleLabel = nullptr;          // Shows the active course title.
    QListWidget* prerequisiteList = nullptr;     // Displays prerequisites with status icons.
    QListWidget* warningsList = nullptr;         // Non-blocking warning display.
    QLabel* warningsTitleLabel = nullptr;        // Header for the warning list.
    QTimer* searchDelayTimer = nullptr;          // Debounce timer for the search box.
};
