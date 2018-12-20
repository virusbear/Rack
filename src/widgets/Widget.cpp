#include "widgets/Widget.hpp"
#include "event.hpp"
#include "context.hpp"
#include <algorithm>


namespace rack {

Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	context()->event->finalizeWidget(this);
	clearChildren();
}

math::Rect Widget::getChildrenBoundingBox() {
	math::Rect bound;
	for (Widget *child : children) {
		if (child == children.front()) {
			bound = child->box;
		}
		else {
			bound = bound.expand(child->box);
		}
	}
	return bound;
}

math::Vec Widget::getRelativeOffset(math::Vec v, Widget *relative) {
	if (this == relative) {
		return v;
	}
	v = v.plus(box.pos);
	if (parent) {
		v = parent->getRelativeOffset(v, relative);
	}
	return v;
}

math::Rect Widget::getViewport(math::Rect r) {
	math::Rect bound;
	if (parent) {
		bound = parent->getViewport(box);
	}
	else {
		bound = box;
	}
	bound.pos = bound.pos.minus(box.pos);
	return r.clamp(bound);
}

void Widget::addChild(Widget *widget) {
	assert(!widget->parent);
	widget->parent = this;
	children.push_back(widget);
}

void Widget::removeChild(Widget *widget) {
	assert(widget->parent == this);
	auto it = std::find(children.begin(), children.end(), widget);
	assert(it != children.end());
	children.erase(it);
	widget->parent = NULL;
}

void Widget::clearChildren() {
	for (Widget *child : children) {
		child->parent = NULL;
		delete child;
	}
	children.clear();
}

void Widget::step() {
	for (auto it = children.begin(); it != children.end();) {
		Widget *child = *it;
		// Delete children if a delete is requested
		if (child->requestedDelete) {
			it = children.erase(it);
			child->parent = NULL;
			delete child;
			continue;
		}

		child->step();
		it++;
	}
}

void Widget::draw(NVGcontext *vg) {
	for (Widget *child : children) {
		// Don't draw if invisible
		if (!child->visible)
			continue;
		// Don't draw if child is outside bounding box
		if (!box.zeroPos().intersects(child->box))
			continue;

		nvgSave(vg);
		nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
		child->draw(vg);

		// Draw red hitboxes
		// if (context()->event->hoveredWidget == child) {
		// 	nvgBeginPath(vg);
		// 	nvgRect(vg, 0, 0, child->box.size.x, child->box.size.y);
		// 	nvgFillColor(vg, nvgRGBAf(1, 0, 0, 0.5));
		// 	nvgFill(vg);
		// }

		nvgRestore(vg);
	}
}


} // namespace rack
