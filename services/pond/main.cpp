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

#include "Mouse.h"
#include "Display.h"
#include "Server.h"
#include "Window.h"
#include "DecorationWindow.h"
#include <poll.h>
#include <unistd.h>

int main() {
	auto* display = new Display;
	auto* server = new Server;
	auto* main_window = new Window(display);
	auto* mouse = new Mouse(main_window);

	display->repaint();

	struct pollfd polls[2];
	polls[0].fd = mouse->fd();
	polls[0].events = POLLIN;
	polls[1].fd = server->fd();
	polls[1].events = POLLIN;

	if(!fork()) {
		char* argv[] = {NULL};
		char* envp[] = {NULL};
		execve("/bin/pondtest", argv, envp);
	}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
	while(true) {
		poll(polls, 2, 16);
		mouse->update();
		server->handle_packets();
		display->repaint();
		display->flip_buffers();
	}
#pragma clang diagnostic pop

}