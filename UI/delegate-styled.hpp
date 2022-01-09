#include <QStyledItemDelegate>
#include <QApplication>

class PopupItemDelegate : public QStyledItemDelegate {
public:
	using QStyledItemDelegate::QStyledItemDelegate;
	QSize sizeHint(const QStyleOptionViewItem &option,
		       const QModelIndex &index) const override
	{
		if (!index.isValid())
			return QSize();

		QSize s = QStyledItemDelegate::sizeHint(option, index);
		return s;
	}
};