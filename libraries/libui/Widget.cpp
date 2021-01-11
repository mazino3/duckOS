/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include "Widget.h"
#include "libui.h"

using namespace UI;

Dimensions Widget::preferred_size() {
	return {1, 1};
}

Dimensions Widget::current_size() {
	if(!_initialized_size)
		_size = preferred_size();
	return _size;
}

void Widget::repaint() {
	if(_window) {
		do_repaint(_window->framebuffer);
		_window->invalidate();
	}
}

bool Widget::on_keyboard(Pond::KeyEvent evt) {
	return false;
}

bool Widget::on_mouse(Pond::MouseEvent evt) {
	return false;
}

Widget* Widget::parent() {
	return _parent;
}

Window* Widget::parent_window() {
	return _parent_window;
}

void Widget::add_child(Widget* child) {
	if(child->parent() || child->parent_window())
		return;
	children.push_back(child);
	child->set_parent(this);
	on_child_added(child);
}

void Widget::set_position(const Point& position) {
	if(_window)
		_window->set_position(position.x, position.y);
	_position = position;
}

Point Widget::position() {
	return _position;
}

void Widget::set_window(UI::Window* window) {
	if(_parent || _parent_window)
		return;

	_parent_window = window;
	_size = preferred_size();
	_window = pond_context->create_window(window->_window, _position.x, _position.y, _size.width, _size.height);
	__register_widget(this, _window->id);
	repaint();
	for(auto& child : children)
		child->parent_window_created();
}

void Widget::set_parent(UI::Widget* widget) {
	if(_parent || _parent_window)
		return;

	_parent = widget;

	if(widget->_window) {
		_size = preferred_size();
		_window = pond_context->create_window(widget->_window, _position.x, _position.y, _size.width, _size.height);
		__register_widget(this, _window->id);
		repaint();
		for(auto& child : children)
			child->parent_window_created();
	}
}

void Widget::update_size() {
	_size = preferred_size();

	if(_window) {
		_window->resize(_size.width, _size.height);
		repaint();
	}

	if(_parent)
		_parent->update_size();

	if(_parent_window)
		_parent_window->resize(_size.width, _size.height);
}

void Widget::do_repaint(Image& framebuffer) {

}

void Widget::parent_window_created() {
	_size = preferred_size();
	_window = pond_context->create_window(_parent ? _parent->_window : _parent_window->_window, _position.x, _position.y, _size.width, _size.height);
	__register_widget(this, _window->id);
	repaint();
	for(auto& child : children)
		child->parent_window_created();
}

void Widget::on_child_added(UI::Widget* child) {

}