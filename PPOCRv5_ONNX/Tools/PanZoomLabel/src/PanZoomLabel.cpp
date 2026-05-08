#include "PanZoomLabel.h"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <algorithm>

static double safeDiv(double a, double b, double fallback)
{
	return (b == 0.0) ? fallback : (a / b);
}

PanZoomLabel::PanZoomLabel(QWidget* parent)
	: QLabel(parent)
{
	setMouseTracking(true);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setScaledContents(false);
	setMinimumSize(1, 1);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

void PanZoomLabel::setOverlayPaths(const QVector<QPainterPath>& pathsImage)
{
	_overlayPathsImage = pathsImage;
	update();
}

void PanZoomLabel::clearOverlayPaths()
{
	_overlayPathsImage.clear();
	update();
}

void PanZoomLabel::setRoiMode(bool enabled)
{
	_roiMode = enabled;

	_dragging = false;
	unsetCursor();

	_activeHandle = RoiHandle::None;
	_roiState = RoiEditState::Idle;
	_roiPreviewImageRect = QRectF();
	_activeRoiImageRect = QRectF();

	update();
}

void PanZoomLabel::clearAllRois()
{
	_roiImageRects.clear();
	cancelActiveRoi();
}

void PanZoomLabel::cancelActiveRoi()
{
	_roiState = RoiEditState::Idle;
	_activeHandle = RoiHandle::None;

	_roiFirstPointImage = QPointF();
	_roiPreviewImageRect = QRectF();
	_activeRoiImageRect = QRectF();

	unsetCursor();
	update();
}

void PanZoomLabel::setPixmap(const QPixmap& pixmap)
{
	_src = pixmap;
	fitToView();
}

void PanZoomLabel::setPixmap(const QPixmap& pixmap, bool resetView)
{
	_src = pixmap;
	if (resetView || _scale <= 0.0)
	{
		fitToView();
		return;
	}

	_minScale = calcFitMinScale();
	_scale = std::clamp(_scale, _minScale, _maxScale);
	ensureValidOffset();
	update();
}

void PanZoomLabel::resetView()
{
	_scale = 1.0;

	if (_src.isNull())
	{
		_offset = QPointF(0, 0);
		update();
		return;
	}

	_minScale = calcFitMinScale();
	_scale = std::max(_scale, _minScale);

	const QSizeF scaledSize = QSizeF(_src.size()) * _scale;
	_offset = QPointF((width() - scaledSize.width()) * 0.5, (height() - scaledSize.height()) * 0.5);

	ensureValidOffset();
	update();
}

void PanZoomLabel::fitToView()
{
	if (_src.isNull())
	{
		_scale = 1.0;
		_offset = QPointF(0, 0);
		update();
		return;
	}

	_minScale = calcFitMinScale();
	_scale = _minScale;

	const QSizeF scaledSize = QSizeF(_src.size()) * _scale;
	_offset = QPointF((width() - scaledSize.width()) * 0.5, (height() - scaledSize.height()) * 0.5);

	ensureValidOffset();
	update();
}

PanZoomLabel::ViewState PanZoomLabel::viewState() const
{
	ViewState s;
	s.scale = _scale;
	s.offset = _offset;
	s.valid = !_src.isNull();
	return s;
}

void PanZoomLabel::applyViewState(const ViewState& state)
{
	if (_src.isNull() || !state.valid)
		return;

	_minScale = calcFitMinScale();
	_scale = std::clamp(state.scale, _minScale, _maxScale);
	_offset = state.offset;
	ensureValidOffset();
	update();
}

void PanZoomLabel::paintEvent(QPaintEvent* e)
{
	QLabel::paintEvent(e);

	if (_src.isNull())
		return;

	QPainter p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);

	// 画图
	p.save();
	p.translate(_offset);
	p.scale(_scale, _scale);
	p.drawPixmap(0, 0, _src);
	p.restore();

	// ROI 绘制（叠加层）
	QPen penPreview(QColor(0, 255, 0), 2, Qt::DashLine);
	QPen penRoi(_roiStrokeColor, 2, Qt::SolidLine);
	QBrush fill(_roiFillColor);
	auto drawHandles = [&](const QRectF& rW)
	{
		const double s = 8.0;
		const QRectF tl(rW.topLeft() - QPointF(s / 2.0, s / 2.0), QSizeF(s, s));
		const QRectF tr(rW.topRight() - QPointF(s / 2.0, s / 2.0), QSizeF(s, s));
		const QRectF bl(rW.bottomLeft() - QPointF(s / 2.0, s / 2.0), QSizeF(s, s));
		const QRectF br(rW.bottomRight() - QPointF(s / 2.0, s / 2.0), QSizeF(s, s));

		p.setPen(QPen(Qt::white, 1));
		p.setBrush(QBrush(QColor(0, 0, 0, 180)));
		p.drawRect(tl);
		p.drawRect(tr);
		p.drawRect(bl);
		p.drawRect(br);
	};

	// 已确认 ROI（多个）
	p.setPen(penRoi);
	p.setBrush(fill);
	for (const QRectF& rImg : _roiImageRects)
	{
		if (!rImg.isValid())
			continue;

		const QRectF rW = imageToWidget(rImg).normalized();
		p.drawRect(rW);
	}

	// 当前预览框
	if (_roiMode && _roiState == RoiEditState::FirstPointSet && _roiPreviewImageRect.isValid())
	{
		const QRectF rW = imageToWidget(_roiPreviewImageRect).normalized();
		p.setPen(penPreview);
		p.setBrush(Qt::NoBrush);
		p.drawRect(rW);
	}

	// 当前工作 ROI（未确认）
	if (_roiMode && _roiState == RoiEditState::Editing && _activeRoiImageRect.isValid())
	{
		const QRectF rW = imageToWidget(_activeRoiImageRect).normalized();
		p.setPen(penRoi);
		p.setBrush(fill);
		p.drawRect(rW);

		drawHandles(rW);
	}

	// 轮廓叠加（图像坐标路径 -> widget）
	if (!_overlayPathsImage.isEmpty())
	{
		QPen penContour(QColor(0, 255, 255), 2, Qt::SolidLine);
		penContour.setCosmetic(true); // 线宽不随缩放变粗
		p.setPen(penContour);
		p.setBrush(Qt::NoBrush);

		QTransform t;
		t.translate(_offset.x(), _offset.y());
		t.scale(_scale, _scale);

		for (const auto& pathImg : _overlayPathsImage)
		{
			p.drawPath(t.map(pathImg));
		}
	}

	// 绕过 ROI 绘制（屏蔽区域，可能有重叠）
	if (!_shieldRoisImage.isEmpty())
	{
		QBrush brushShield(QColor(255, 255, 255, 128));
		p.setBrush(brushShield);
		p.setPen(Qt::NoPen);

		for (const QRectF& rImg : _shieldRoisImage)
		{
			if (!rImg.isValid())
				continue;

			const QRectF rW = imageToWidget(rImg).normalized();
			p.drawRect(rW);
		}
	}
}

void PanZoomLabel::wheelEvent(QWheelEvent* e)
{
	if (_src.isNull())
	{
		e->ignore();
		return;
	}

	_minScale = calcFitMinScale();

	const int delta = e->angleDelta().y();
	if (delta == 0)
	{
		e->accept();
		return;
	}

	const QPointF anchor = e->position();
	const QPointF imgBefore = widgetToImage(anchor);

	const double step = 1.15;
	double newScale = _scale * (delta > 0 ? step : (1.0 / step));
	newScale = std::clamp(newScale, _minScale, _maxScale);
	if (newScale == _scale)
	{
		e->accept();
		return;
	}

	_scale = newScale;
	_offset = anchor - QPointF(imgBefore.x() * _scale, imgBefore.y() * _scale);

	ensureValidOffset();
	update();
	e->accept();
}

void PanZoomLabel::mousePressEvent(QMouseEvent* e)
{
	if (_src.isNull())
	{
		QLabel::mousePressEvent(e);
		return;
	}

	// 右键：确认当前工作 ROI -> 加入列表 -> 发信号（多个 ROI）
	if (_roiMode && e->button() == Qt::RightButton)
	{
		if (_roiState == RoiEditState::Editing && _activeRoiImageRect.isValid())
		{
			QRectF r = _activeRoiImageRect.normalized();
			normalizeRoi(r);
			clampRoiToImage(r);

			_roiImageRects.push_back(r);

			// 确认后锁定：当前这个 ROI 不再可编辑（通过清空工作状态实现）
			cancelActiveRoi();

			emit roiRectsConfirmedChanged(_roiImageRects);

			// 可选：确认一次就退出 ROI 模式（如果你希望每次确认都回到拖图模式）
			setRoiMode(false);
		}

		e->accept();
		return;
	}

	// ROI 模式：左键绘制/编辑当前工作 ROI
	if (_roiMode && e->button() == Qt::LeftButton)
	{
		const QPointF pImg = widgetToImage(e->position());

		if (_roiState == RoiEditState::Idle)
		{
			_roiFirstPointImage = pImg;
			_roiPreviewImageRect = QRectF(_roiFirstPointImage, _roiFirstPointImage);
			_roiState = RoiEditState::FirstPointSet;
			setCursor(Qt::CrossCursor);
			update();
			e->accept();
			return;
		}

		if (_roiState == RoiEditState::FirstPointSet)
		{
			_roiPreviewImageRect = QRectF(_roiFirstPointImage, pImg).normalized();
			normalizeRoi(_roiPreviewImageRect);
			clampRoiToImage(_roiPreviewImageRect);

			_activeRoiImageRect = _roiPreviewImageRect;
			_roiPreviewImageRect = QRectF();
			_roiState = RoiEditState::Editing;

			unsetCursor();
			update();
			e->accept();
			return;
		}

		// Editing：命中测试（只针对当前工作 ROI）
		_activeHandle = hitTestActiveRoiHandle(e->position());
		if (_activeHandle != RoiHandle::None)
		{
			_dragStartMouseImage = pImg;
			_dragStartRoiImage = _activeRoiImageRect;
			e->accept();
			return;
		}

		e->accept();
		return;
	}

	// 非 ROI 模式：左键拖动图片
	if (!_roiMode && e->button() == Qt::LeftButton)
	{
		_dragging = true;
		_lastMousePos = e->pos();
		setCursor(Qt::ClosedHandCursor);
		e->accept();
		return;
	}

	QLabel::mousePressEvent(e);
}

void PanZoomLabel::mouseMoveEvent(QMouseEvent* e)
{
	if (_src.isNull())
	{
		QLabel::mouseMoveEvent(e);
		return;
	}

	// ROI 模式绘制预览
	if (_roiMode && _roiState == RoiEditState::FirstPointSet)
	{
		const QPointF pImg = widgetToImage(e->position());
		_roiPreviewImageRect = QRectF(_roiFirstPointImage, pImg).normalized();
		normalizeRoi(_roiPreviewImageRect);
		clampRoiToImage(_roiPreviewImageRect);
		update();
		e->accept();
		return;
	}

	// ROI 模式：编辑当前工作 ROI
	if (_roiMode && _roiState == RoiEditState::Editing && _activeHandle != RoiHandle::None)
	{
		const QPointF curImg = widgetToImage(e->position());
		const QPointF d = curImg - _dragStartMouseImage;

		QRectF r = _dragStartRoiImage;

		switch (_activeHandle)
		{
		case RoiHandle::Move:
			r.translate(d);
			break;
		case RoiHandle::TopLeft:
			r.setTopLeft(r.topLeft() + d);
			break;
		case RoiHandle::TopRight:
			r.setTopRight(r.topRight() + d);
			break;
		case RoiHandle::BottomLeft:
			r.setBottomLeft(r.bottomLeft() + d);
			break;
		case RoiHandle::BottomRight:
			r.setBottomRight(r.bottomRight() + d);
			break;
		default:
			break;
		}

		r = r.normalized();
		normalizeRoi(r);
		clampRoiToImage(r);

		_activeRoiImageRect = r;
		update();
		e->accept();
		return;
	}

	// ROI 模式 hover 光标（仅当前工作 ROI）
	if (_roiMode && _roiState == RoiEditState::Editing)
	{
		const RoiHandle h = hitTestActiveRoiHandle(e->position());
		switch (h)
		{
		case RoiHandle::Move: setCursor(Qt::SizeAllCursor); break;
		case RoiHandle::TopLeft:
		case RoiHandle::BottomRight: setCursor(Qt::SizeFDiagCursor); break;
		case RoiHandle::TopRight:
		case RoiHandle::BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
		default: unsetCursor(); break;
		}
		e->accept();
		return;
	}

	// 图片拖动
	if (!_roiMode && _dragging)
	{
		const QPoint delta = e->pos() - _lastMousePos;
		_lastMousePos = e->pos();

		_offset += QPointF(delta.x(), delta.y());
		ensureValidOffset();
		update();
		e->accept();
		return;
	}

	QLabel::mouseMoveEvent(e);
}

void PanZoomLabel::mouseReleaseEvent(QMouseEvent* e)
{
	if (!_roiMode && e->button() == Qt::LeftButton && _dragging)
	{
		_dragging = false;
		unsetCursor();
		e->accept();
		return;
	}

	if (_roiMode && e->button() == Qt::LeftButton)
	{
		_activeHandle = RoiHandle::None;
		e->accept();
		return;
	}

	QLabel::mouseReleaseEvent(e);
}

void PanZoomLabel::resizeEvent(QResizeEvent* e)
{
	QLabel::resizeEvent(e);

	if (!_src.isNull())
	{
		_minScale = calcFitMinScale();
		_scale = std::clamp(_scale, _minScale, _maxScale);
	}

	ensureValidOffset();
	update();
}

QPointF PanZoomLabel::widgetToImage(const QPointF& pWidget) const
{
	return (pWidget - _offset) / _scale;
}

QPointF PanZoomLabel::imageToWidget(const QPointF& pImage) const
{
	return _offset + pImage * _scale;
}

QRectF PanZoomLabel::imageToWidget(const QRectF& rImage) const
{
	return QRectF(imageToWidget(rImage.topLeft()), imageToWidget(rImage.bottomRight())).normalized();
}

double PanZoomLabel::calcFitMinScale() const
{
	if (_src.isNull() || width() <= 0 || height() <= 0)
		return 0.05;

	const double sx = safeDiv(static_cast<double>(width()), static_cast<double>(_src.width()), 1.0);
	const double sy = safeDiv(static_cast<double>(height()), static_cast<double>(_src.height()), 1.0);
	return std::max(0.0001, std::min(sx, sy));
}

PanZoomLabel::RoiHandle PanZoomLabel::hitTestActiveRoiHandle(const QPointF& posWidget) const
{
	if (!_activeRoiImageRect.isValid())
		return RoiHandle::None;

	const QRectF rW = imageToWidget(_activeRoiImageRect).normalized();

	const double s = 10.0;
	const QRectF tl(rW.topLeft() - QPointF(s, s), QSizeF(2 * s, 2 * s));
	const QRectF tr(rW.topRight() - QPointF(s, s), QSizeF(2 * s, 2 * s));
	const QRectF bl(rW.bottomLeft() - QPointF(s, s), QSizeF(2 * s, 2 * s));
	const QRectF br(rW.bottomRight() - QPointF(s, s), QSizeF(2 * s, 2 * s));

	if (tl.contains(posWidget)) return RoiHandle::TopLeft;
	if (tr.contains(posWidget)) return RoiHandle::TopRight;
	if (bl.contains(posWidget)) return RoiHandle::BottomLeft;
	if (br.contains(posWidget)) return RoiHandle::BottomRight;

	if (rW.contains(posWidget))
		return RoiHandle::Move;

	return RoiHandle::None;
}

void PanZoomLabel::normalizeRoi(QRectF& r) const
{
	const double minW = 2.0;
	const double minH = 2.0;

	r = r.normalized();
	if (r.width() < minW)  r.setWidth(minW);
	if (r.height() < minH) r.setHeight(minH);
}

void PanZoomLabel::clampRoiToImage(QRectF& r) const
{
	if (_src.isNull())
		return;

	const QRectF imgBounds(0.0, 0.0, static_cast<double>(_src.width()), static_cast<double>(_src.height()));

	if (r.left() < imgBounds.left()) r.translate(imgBounds.left() - r.left(), 0);
	if (r.top() < imgBounds.top()) r.translate(0, imgBounds.top() - r.top());
	if (r.right() > imgBounds.right()) r.translate(imgBounds.right() - r.right(), 0);
	if (r.bottom() > imgBounds.bottom()) r.translate(0, imgBounds.bottom() - r.bottom());

	r = r.intersected(imgBounds);
}

void PanZoomLabel::ensureValidOffset()
{
	if (_src.isNull())
		return;

	const QSizeF scaledSize = QSizeF(_src.size()) * _scale;

	const double minX = std::min(0.0, width() - scaledSize.width());
	const double minY = std::min(0.0, height() - scaledSize.height());

	if (scaledSize.width() <= width())
		_offset.setX((width() - scaledSize.width()) * 0.5);
	else
		_offset.setX(std::clamp(_offset.x(), minX, 0.0));

	if (scaledSize.height() <= height())
		_offset.setY((height() - scaledSize.height()) * 0.5);
	else
		_offset.setY(std::clamp(_offset.y(), minY, 0.0));
}

//void PanZoomLabel::setRoiDrawColor(const QColor& stroke, const QColor& fill)
//{
//	_roiStrokeColor = stroke;
//	_roiFillColor = fill;
//	update();
//}

void PanZoomLabel::setCreateRois(const QVector<QRectF>& roisImage)
{
	_createRoisImage = roisImage;
	update();
}

void PanZoomLabel::setShieldRois(const QVector<QRectF>& roisImage)
{
	_shieldRoisImage = roisImage;
	update();
}
	