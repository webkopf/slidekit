
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

static gboolean experimental = false;
static gboolean page_cache = false;
static gboolean no_autoplay = false;
static gboolean smooth_scrolling = false;
static gboolean fullscreen_enabled = false;


gboolean videoPlaying = false;
char htmlFilePath[4096];
char* basePath;

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


long determineRemainingMilliSecs(char* omx_status_string){

	char* ptr = strtok(omx_status_string, " ");
	char* durationMicroSecsStr = strtok(NULL, "\n");
	printf("%s\n", durationMicroSecsStr);
	strtok(NULL, " ");
	char* positionMicroSecsStr = strtok(NULL, "\n");

	long durationMicroSecs = strtol(durationMicroSecsStr, NULL, 10);
	long positionMicroSecs = strtol(positionMicroSecsStr, NULL, 10);

	long remainingMilliSecs = (durationMicroSecs - positionMicroSecs) / 1000;

	return remainingMilliSecs;
}

static void play_active_video() {

//#define OMX_DISABLED
#ifndef OMX_DISABLED
	WebKitDOMDocument* domDocument = webkit_web_view_get_dom_document(web_view);

	WebKitDOMElement* sourceElem = webkit_dom_document_query_selector(
			domDocument, ".active video source", NULL);
	if (sourceElem != NULL) {
		gchar* srcAttr = webkit_dom_element_get_attribute(sourceElem, "src");

		char invocation[512];
		sprintf(invocation, "xterm -fn fixed -fullscreen -maximized -bg black -fg black -e omxplayer %s/%s",
				basePath, srcAttr);


		printf("omx-invoke: %s\n", invocation);

//		gchar* invocation = "xterm -fn fixed -fullscreen -maximized -bg black -fg black -e omxplayer /home/pi/mvz/tvp-affenwelten-e01-br-1080p.mp4";
		popen(invocation, "r");

		sleep(3);

		gchar* dbusInvocation = "/usr/local/share/slidekit/omx-dbus-control.sh org.mpris.MediaPlayer2.omxplayer status";

		int ret;
		FILE *stream;
		char buf[128];
		char omx_status[1024];

		if ((stream = popen(dbusInvocation, "r")) == NULL) {
			perror("popen: popen() failed");
		}

		while(fgets(buf, 128, stream) != NULL){
			strcat(omx_status, buf);
		}


		ret = pclose(stream);
		printf("(ls returned %d.)\n", ret);

		printf("omx_status: %s\n", omx_status);

//		char status[] = "Duration: 2585040000\nPosition: 1271832\nPaused: false";

		long remainingMilliSecs = determineRemainingMilliSecs(omx_status);

		WebKitWebFrame* frame = webkit_web_view_get_main_frame(web_view);
		JSGlobalContextRef ctx = webkit_web_frame_get_global_context(frame);

		JSObjectRef global = JSContextGetGlobalObject(ctx);

		char scriptStr[64];
		sprintf(scriptStr,
			"setTimeout(nextSlideStep, %ld);", remainingMilliSecs);


		JSStringRef script = JSStringCreateWithUTF8CString(scriptStr);
		JSEvaluateScript(ctx, script, NULL, NULL, 0, NULL);

		videoPlaying = TRUE;
	}

#endif
}

static void stop_active_video() {
	if(videoPlaying){
		popen("pkill xterm", "r");
		videoPlaying = false;
	}
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

	WebKitDOMDocument* domDocument = webkit_web_view_get_dom_document(web_view);

	WebKitDOMElement* activeSlideElem = webkit_dom_document_query_selector(domDocument, ".active", NULL);
	gchar* className = webkit_dom_element_get_attribute(activeSlideElem, "class");

	g_print("on_before_next_slide, active className: %s\n", className);

	stop_active_video();

	return NULL;
}

JSValueRef on_after_next_slide(JSContextRef ctx, JSObjectRef function,
		JSObjectRef thisObject, size_t argumentCount,
		const JSValueRef arguments[], JSValueRef *exception) {

	WebKitDOMDocument* domDocument = webkit_web_view_get_dom_document(web_view);

	WebKitDOMElement* activeSlideElem = webkit_dom_document_query_selector(domDocument, ".active", NULL);
	gchar* className = webkit_dom_element_get_attribute(activeSlideElem, "class");

	g_print("on_after_next_slide, active className: %s\n", className);

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

	register_javascript_function("on_before_next_slide", on_before_next_slide);
	register_javascript_function("on_after_next_slide", on_after_next_slide);

	WebKitDOMDocument* domDocument = webkit_web_view_get_dom_document(web_view);
	WebKitDOMElement* body = (WebKitDOMElement*) webkit_dom_document_get_body(domDocument);
	webkit_dom_element_set_attribute(body, "class", "raspberryMode", NULL);

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

static WebKitWebView* createWebView() {

	char url[4096];
	strcpy(url, "file://");
	strcat(url, htmlFilePath);

	WebKitWebSettings* settings;

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

	web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

	settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));

	g_object_set(G_OBJECT(settings), "enable-private-browsing", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-file-access-from-file-uris", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-universal-access-from-file-uris", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-spatial-navigation", false, NULL);
	g_object_set(G_OBJECT(settings), "default-encoding", "utf-8", NULL);
	g_object_set(G_OBJECT(settings), "enable-page-cache", page_cache, NULL);
	g_object_set(G_OBJECT(settings), "enable-plugins", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-site-specific-quirks", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-accelerated-compositing", TRUE, NULL);
	g_object_set(G_OBJECT(settings), "enable-webgl", FALSE, NULL);
	g_object_set(G_OBJECT(settings), "enable-webaudio", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-media-stream", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-mediasource", experimental, NULL);
	g_object_set(G_OBJECT(settings), "enable-smooth-scrolling",
			smooth_scrolling, NULL);

	g_object_set(G_OBJECT(settings), "media-playback-requires-user-gesture",
			no_autoplay, NULL);

	g_object_set(G_OBJECT(settings), "enable-scripts", TRUE, NULL);

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

void determineBasePath(char* htmlFilePath) {
	int urlLength = strlen(htmlFilePath);

	int lastSlashIdx = urlLength;
	for (; lastSlashIdx > 0; lastSlashIdx--) {
		if (htmlFilePath[lastSlashIdx] == '/') {
			break;
		}
	}

	basePath = (char *) malloc((lastSlashIdx + 1) * sizeof(char));
	memcpy(basePath, htmlFilePath, lastSlashIdx);
	basePath[lastSlashIdx] = '\0';
}


int main(int argc, char* argv[]) {

	if (argv[1] == NULL) {
		//=> no arg provided, show slidekit.html from the web-folder
		long size;
		char *buf;
		char *cwd;
		size = pathconf(".", _PC_PATH_MAX);

		if ((buf = (char *) malloc((size_t) size)) != NULL) {
			cwd = getcwd(buf, (size_t) size);
		}

		htmlFilePath[4096];
		strcpy(htmlFilePath, cwd);
		strcat(htmlFilePath, "/web/slidekit.html");
	}
	else{
		strcpy(htmlFilePath, argv[1]);
	}

	determineBasePath(htmlFilePath);

	printf("htmlFilePath: %s\n", htmlFilePath);
	printf("basePath: %s\n", basePath);

	gtk_init(&argc, &argv);

	webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

	createWebView();

//	signal(SIGCHLD, signal_catcher);
	gtk_main();
//
	printf("normal exit\n");
	return 0;

}
