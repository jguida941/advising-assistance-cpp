#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <string>

// Simple list model that exposes the catalog's sorted course IDs to views.
class CourseListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit CourseListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;  // Number of rows shown in the list view.
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;  // Display text for each row.

    void setCourseIds(std::vector<std::string> courseIds);  // Replace the list backing the view.
    QString courseIdForRow(int row) const;                  // Fetch the ID for selection helpers.

private:
    std::vector<std::string> courseIds;  // Keeps the sorted IDs returned from the catalog.
};
