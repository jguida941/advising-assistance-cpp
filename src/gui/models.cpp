#include "gui/models.hpp"

#include <utility>

CourseListModel::CourseListModel(QObject* parent)
    : QAbstractListModel(parent) {}

// Returns how many IDs the view should render. Parent checks allow Qt to query children.
int CourseListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(courseIds.size());  // One row per course ID.
}

// Provides the text shown in each row of the QListView.
QVariant CourseListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }
    if (index.row() < 0 || index.row() >= static_cast<int>(courseIds.size())) {
        return {};
    }
    return QString::fromStdString(courseIds[index.row()]);  // Display text is just the course ID.
}

// Replaces the backing vector with a freshly sorted list from the catalog core.
void CourseListModel::setCourseIds(std::vector<std::string> ids) {
    beginResetModel();
    courseIds = std::move(ids);
    endResetModel();
}

// Convenience helper so other widgets can resolve the ID for a given row index.
QString CourseListModel::courseIdForRow(int row) const {
    if (row < 0 || row >= static_cast<int>(courseIds.size())) {
        return {};
    }
    return QString::fromStdString(courseIds[row]);  // Helper for selection syncing.
}
