#pragma once

#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QPainterPath>

class PanZoomLabel : public QLabel
{
	Q_OBJECT
public:
	explicit PanZoomLabel(QWidget* parent = nullptr);

	struct ViewState
	{
		double scale{ 1.0 };
		QPointF offset{ 0.0, 0.0 };
		bool valid{ false };
	};

	void setPixmap(const QPixmap& pixmap);
	void setPixmap(const QPixmap& pixmap, bool resetView);
	void resetView();
	void fitToView();

	ViewState viewState() const;
	void applyViewState(const ViewState& state);

	void setRoiMode(bool enabled);
	bool roiMode() const { return _roiMode; }

	// 原有：全部 ROI（如你不再需要可逐步淘汰）
	const QVector<QRectF>& roiImageRects() const { return _roiImageRects; }
	void clearAllRois();
	void cancelActiveRoi();

	// 新增：分组 ROI（图像坐标：像素）
	void setCreateRois(const QVector<QRectF>& roisImage);
	void setShieldRois(const QVector<QRectF>& roisImage);

	void setOverlayPaths(const QVector<QPainterPath>& pathsImage);
	void clearOverlayPaths();

signals:
	void roiRectsConfirmedChanged(const QVector<QRectF>& roiImageRects);

protected:
	void paintEvent(QPaintEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;

private:
	void ensureValidOffset();

	QPointF widgetToImage(const QPointF& pWidget) const;
	QPointF imageToWidget(const QPointF& pImage) const;
	QRectF  imageToWidget(const QRectF& rImage) const;

	double calcFitMinScale() const;

	enum class RoiEditState { Idle, FirstPointSet, Editing };
	enum class RoiHandle { None, Move, TopLeft, TopRight, BottomLeft, BottomRight };

	RoiHandle hitTestActiveRoiHandle(const QPointF& posWidget) const;
	void normalizeRoi(QRectF& r) const;
	void clampRoiToImage(QRectF& r) const;

private:
	QPixmap _src{};

	double _scale{ 1.0 };
	double _minScale{ 0.05 };
	double _maxScale{ 20.0 };

	QPointF _offset{ 0.0, 0.0 };

	bool _dragging{ false };
	QPoint _lastMousePos{};

	// ROI 状态
	bool _roiMode{ false };
	RoiEditState _roiState{ RoiEditState::Idle };

	// 已确认 ROI（多个，图像坐标）
	QVector<QRectF> _roiImageRects{};

	// 当前工作 ROI（未确认）
	QPointF _roiFirstPointImage{};
	QRectF  _roiPreviewImageRect{};
	QRectF  _activeRoiImageRect{};

	RoiHandle _activeHandle{ RoiHandle::None };
	QPointF _dragStartMouseImage{};
	QRectF  _dragStartRoiImage{};

	// 叠加轮廓（图像坐标路径）
	QVector<QPainterPath> _overlayPathsImage{};

	QColor _roiStrokeColor{ QColor(255, 0, 0) };  // 默认红
	QColor _roiFillColor{ QColor(255, 0, 0, 40) };

	// 新增：分组 ROI
	QVector<QRectF> _createRoisImage{};
	QVector<QRectF> _shieldRoisImage{};
};