#pragma once

#include "catalog/catalog.hpp"
#include "gui/models.hpp"

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

// Qt dashboard window that mirrors the CLI features with a point-and-click UI.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Catalog catalog, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // Prompts for a catalog file and loads it into the shared catalog instance.
    void openCatalog();
    // Re-runs the load using the most recently opened file so edits are picked up.
   void reloadCatalog();
    // Updates the detail pane when the list view selection changes.
    void handleCourseSelection(const QModelIndex& index);
    // Debounces user typing in the search field before kicking off a lookup.
    void handleSearchEdited(const QString& text);
    // Looks up the active search text and syncs the selection.
    void performSearch();
    // Double-clicking a prerequisite jumps straight to that course.
    void handlePrerequisiteActivated(QListWidgetItem* item);
    // Displays any prerequisites that were missing in the source CSV.
   void showMissingPrerequisites();

private:
    // Builds the menu bar actions for file handling and warnings.
    void createMenus();
    // Lays out the search field, list view, detail pane, and warnings list.
    void createLayout();
    // Core helper that loads the given CSV path and refreshes UI state.
    void loadCatalogFromPath(const QString& path);
    // Fills the right-hand pane with details about the active course.
    void populateCourseDetails(const Course* course);
    // Replaces the list model contents with the current sorted IDs.
    void refreshCourseList();
    // Updates the status bar with the last load result.
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
