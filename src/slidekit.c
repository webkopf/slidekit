/*                Minimal Kiosk Browser

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version (see http://www.gnu.org/licenses/ ).

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 Copyright 2013 Ralph Glass (Minimal Web Browser base code)
 Copyright 2013-2015 Guenter Kreidl (Minimal Kiosk Browser)

 Version 1.6.9 using gtk+3 and webkitgtk-3.0
 */

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <JavaScriptCore/JavaScript.h>

#include "utils.h"

static gboolean javascript = false;
static gboolean private_browsing = true;
static gboolean experimental = false;
static gboolean full_zoom = false;
static gboolean page_cache = false;
static gboolean no_autoplay = false;
static gboolean smooth_scrolling = false;
static gboolean fullscreen_enabled = false;

char base_path[512];
char* base_uri;

static WebKitWebView* web_view;

static pid_t xterm_pid;

static void destroy(GtkWidget* widget, gpointer* data) {

	//stop_active_video();
	gtk_main_quit();
}

static void toggleOmx(GtkWidget* item, WebKitWebView* webView) {
//	useOMX = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item));

//	int pid = 0;
//	pid = fork();
//	if (r == 0) {
//		char *execArgs[] = {"omxplayer", "--win", "50,50,300,300", "/home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4", NULL};
//		execvp("omxplayer", execArgs);
//	}

//	int pid = fork();

//	execl("/bin/sh", "sh", "-c", "omxplayer", "-win", "50,50,300,300", "/home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4", NULL);

//	pid_t pid = popen2("omxplayer", "--win", "50,50,300,300", "/home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4");

}

static void reload(GtkWidget* item, WebKitWebView* webView) {
	webkit_web_view_reload(webView);
}

static void toggle_fullscreen(GtkWidget* window, WebKitWebView* webView) {
	fullscreen_enabled = !fullscreen_enabled;
	if (fullscreen_enabled) {
		gtk_window_fullscreen(GTK_WINDOW(window));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(window));
	}
}

static void play_active_video() {
	WebKitDOMDocument* domDocument = webkit_web_view_get_dom_document(web_view);

	WebKitDOMElement* sourceElem = webkit_dom_document_query_selector(
			domDocument, ".active video source", NULL);
	if (sourceElem != NULL) {
		gchar* srcAttr = webkit_dom_element_get_attribute(sourceElem, "src");
		char videoUri[512];
		strcpy(videoUri, base_path);
		strcat(videoUri, "/");
		strcat(videoUri, srcAttr);
		printf("videoPath: %s\n", videoUri);

		//-fullscreen -maximized
		//

		gchar* invocation = "xterm -fn fixed -fullscreen -maximized -bg black -fg black -e omxplayer /home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4";
		popen(invocation, "r");

		sleep(1);

		gchar* dbusInvocation = "./dbuscontrolm.sh org.mpris.MediaPlayer2.omxplayer status";
//		gchar* dbusInvocation = "ls -la 2>&1";
//		int status;
//		FILE* stream;
//		char buffer[40];
//
//		if ((stream = popen("ls -la", "r")) == NULL) {
//			perror("popen() failed on dbusInvocation");
//			printf("no stream?");
//		}
//
//		printf("dbus invoked");

		int status;
		FILE *stream;
		char buffer[1024];

		if ((stream = popen(dbusInvocation, "r")) == NULL) {
			perror("popen: popen() failed");
		}

		while (fgets(buffer, 1024, stream) != NULL){

			printf("%s", buffer);

		}


		status = pclose(stream);
		printf("(ls returned %d.)\n", status);

		//execl("/bin/sh", "sh", "-c", "omxplayer", "-win", "50,50,300,300", "/home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4", NULL);

		//omxplayer /home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4

		//char *command[] ={"xterm", "-fn", "fixed", "-bg", "black", "-fg", "black", "-e", "gnome-calculator", NULL};

//		char *command[] ={"xterm", "-fn", "fixed", "-bg", "black", "-fg", "-fullscreen", "black", "-e", "omxplayer", videoUri, NULL};
//		xterm_pid = popen2(command, NULL, NULL);

		//"-fullscreen", "-maximized",

		//	execl("/bin/sh", "sh", "-c", "omxplayer", "-win", "50,50,300,300", "/home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4", NULL);
	}
}

static void stop_active_video() {
//	if(xterm_pid > 0){
	//execl("./dbuscontrolm.sh", "dbuscontrolm.sh", "org.mpris.MediaPlayer2.omxplayer", "stop", NULL);
//		kill(xterm_pid, SIGTERM);
	popen("pkill xterm", "r");
	xterm_pid = 0;
//	}
}

JSValueRef on_button_clicked(JSContextRef ctx, JSObjectRef function,
		JSObjectRef thisObject, size_t argumentCount,
		const JSValueRef arguments[], JSValueRef *exception) {
	g_print("C-Funktion wurde aufgerufen\n");
	return NULL;
}

JSValueRef on_before_next_slide(JSContextRef ctx, JSObjectRef function,
		JSObjectRef thisObject, size_t argumentCount,
		const JSValueRef arguments[], JSValueRef *exception) {

	g_print("on_before_next_slide\n");

	stop_active_video();

	return NULL;
}

JSValueRef on_after_next_slide(JSContextRef ctx, JSObjectRef function,
		JSObjectRef thisObject, size_t argumentCount,
		const JSValueRef arguments[], JSValueRef *exception) {

	g_print("on_after_next_slide\n");

	play_active_video();

	return NULL;
}

void register_javascript_function(const char *name,
		JSObjectCallAsFunctionCallback callback) {
	WebKitWebFrame *frame = webkit_web_view_get_main_frame(
			WEBKIT_WEB_VIEW(web_view));
	JSContextRef ctx = webkit_web_frame_get_global_context(frame);
	JSObjectRef global = JSContextGetGlobalObject(ctx);
	JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, NULL, callback);
	JSStringRef jsname = JSStringCreateWithUTF8CString(name);
	JSObjectSetProperty(ctx, global, jsname, func, 0, NULL);
	JSStringRelease(jsname);
}

static void on_document_loaded(WebKitWebView* web_view) {
	g_print("on_document_loaded\n");

	WebKitWebFrame* frame = webkit_web_view_get_main_frame(web_view);

	JSGlobalContextRef ctx = webkit_web_frame_get_global_context(frame);

	register_javascript_function("on_before_next_slide", on_before_next_slide);
	register_javascript_function("on_after_next_slide", on_after_next_slide);

//	WebKitDOMHTMLElement* body = webkit_dom_document_get_body(domDocument);

//	webkit_dom_element_set_attribute(body, "style", "background: pink", NULL);

	play_active_video();

}

static gboolean web_key_pressed(GtkWidget* window, GdkEventKey* event,
		WebKitWebView* webView) {

	if (event->state & GDK_CONTROL_MASK) {
		switch (event->keyval) {
		case 'q':
			gtk_widget_destroy(window);
			break;
		case 'f':
			toggle_fullscreen(window, webView);
			break;
		case 'r':
			reload(window, webView);
		}
	}
	return FALSE;
}

static WebKitWebView* createWebView(gchar* url) {
	gboolean js = javascript;
	gboolean fz = full_zoom;

	WebKitWebSettings* settings;

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

	web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

	settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));

	g_object_set(G_OBJECT(settings), "enable-private-browsing",
			private_browsing, NULL);
	g_object_set(G_OBJECT(settings), "enable-file-access-from-file-uris", TRUE,
	NULL);
	g_object_set(G_OBJECT(settings), "enable-universal-access-from-file-uris",
	TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-spatial-navigation", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "default-encoding", "utf-8", NULL);
	g_object_set(G_OBJECT(settings), "enable-page-cache", page_cache, NULL);
	g_object_set(G_OBJECT(settings), "enable-plugins", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-site-specific-quirks", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-accelerated-compositing", TRUE,
	NULL);
	g_object_set(G_OBJECT(settings), "enable-webgl", FALSE, NULL);
	g_object_set(G_OBJECT(settings), "enable-webaudio", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-media-stream", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-mediasource", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-smooth-scrolling",
			smooth_scrolling, NULL);

	g_object_set(G_OBJECT(settings), "media-playback-requires-user-gesture",
			no_autoplay, NULL);

	g_object_set(G_OBJECT(settings), "enable-scripts", TRUE, NULL);
	webkit_web_view_set_full_content_zoom(web_view, fz);

	g_signal_connect(web_view, "document-load-finished",
			G_CALLBACK(on_document_loaded), web_view);

	/*	g_object_set_data(G_OBJECT(window), "fullscreen",
	 g_strjoin(NULL, "f", NULL));
	 gtk_window_fullscreen(GTK_WINDOW(window));*/

	gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(web_view));

	gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	gtk_widget_grab_focus(GTK_WIDGET(web_view));
	gtk_widget_show_all(window);

	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(window, "key-press-event", G_CALLBACK(web_key_pressed),
			web_view);

	if (fullscreen_enabled) {
		gtk_window_fullscreen(GTK_WINDOW(window));
	}

	webkit_web_view_load_uri(web_view, url);
	return web_view;
}

void signal_catcher(int signal) {
	int status;
	int chpid = waitpid(-1, &status, WNOHANG);
}

void determine_base_uri(char* url) {
	int urlLength = strlen(url);

	int lastSlashIdx = urlLength;
	for (; lastSlashIdx > 0; lastSlashIdx--) {
		if (url[lastSlashIdx] == '/') {
			break;
		}
	}

	base_uri = (char *) malloc((lastSlashIdx + 1) * sizeof(char));
	memcpy(base_uri, url, lastSlashIdx);
	base_uri[lastSlashIdx + 1] = '\0';

}

int main(int argc, char* argv[]) {

	char *url = argv[1];
	if (url == NULL) {
		//=> no arg provided, show slidekit.html
		long size;
		char *buf;
		char *cwd;
		size = pathconf(".", _PC_PATH_MAX);

		if ((buf = (char *) malloc((size_t) size)) != NULL) {
			cwd = getcwd(buf, (size_t) size);
		}

		strcpy(base_path, cwd);
		strcat(base_path, "/web");

		char buf2[512];
		strcpy(buf2, "file://");
		strcat(buf2, base_path);
		strcat(buf2, "/slidekit.html#autoplay");

		url = buf2;

		printf("url: %s\n", url);
	}

	determine_base_uri(url);

	printf("base_path: %s\n", base_path);
	printf("base_url: %s\n", base_uri);

	gtk_init(&argc, &argv);

	webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

	createWebView(url);

	signal(SIGCHLD, signal_catcher);
	gtk_main();

	printf("normal exit\n");
	return 0;

}
