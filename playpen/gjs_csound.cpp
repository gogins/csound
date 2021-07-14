#if (defined(WIN32) || defined(_WIN32)) && !defined(SWIG) && !defined(_MSC_VER)
#  define SILENCE_PUBLIC __declspec(dllexport)
#elif defined(__GNUC__) && (__GNUC__ >= 4) /* && !defined(__MACH__) */
#  define SILENCE_PUBLIC        __attribute__ ( (visibility("default")) )
#else
#  define SILENCE_PUBLIC
#endif

#include <string>
#include <csound/csound.h>
#include <csound/csound.hpp>
#include <csound/csound_threaded.hpp>
#include <glib-2.0/glib.h>
#include <glib-2.0/gmodule.h>
#include <libsoup-2.4/libsoup/soup.h>
#include <libsoup-2.4/libsoup/soup-address.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <webkit2/webkit-web-extension.h>

/**
 *
Wrap a subset of CsoundThreaded as a JavaScriptCore WebExtension. The following is the csound.node
signature of Csound.

int result = csound_.Cleanup();
int result = csound_.CompileCsd(csd.c_str());
int result = csound_.CompileCsdText(csd.c_str());
int result = csound_.CompileOrc(csd.c_str());
double value = csound_.EvalCode(code.c_str());
double value = csound_.GetControlChannel(name.c_str(), &result);
int value = csound_.GetKsmps();
std::string value = csound_.GetMetadata(tag.c_str());
int value = csound_.GetNchnls();
int value = csound_.GetScoreTime();
int value = csound_.GetSr();
int value = csound_.GetVersion();
csound_.InputMessage(text.c_str());
bool value = csound_.IsPlaying();
bool value = csound_.IsScorePending();
csound_.Message(text.c_str());
int result = csound_.Perform();
int result = csound_.PerformAndPostProcess();
int result = csound_.ReadScore(sco.c_str());
csound_.Reset();
csound_.RewindScore();
int result = csound_.ScoreEvent(opcode[0], pfields.data(), pfields.size());
csound_.SetChannel(name.c_str(), value);
csound_.SetMetadata(tag.c_str(), value.c_str());
csound_.SetOption(value.c_str());
csound_.SetOutput(filename.c_str(), type.c_str(), format.c_str());
csound_.SetScorePending(value);
int result = csound_.Start();
csound_.Stop();
*/

static CsoundThreaded csound;

extern "C" {

    void destroy_notify_(gpointer data) {
        printf("destroy_notify: %p\n", data);
    }

    void on_gjs_csound_hello() {
        std::printf("Hello from Csound!\n");
    }

    void
    window_object_cleared_callback (WebKitScriptWorld *world,
                                    WebKitWebPage     *web_page,
                                    WebKitFrame       *frame,
                                    gpointer           user_data) 
    {
        JSCContext *js_context = webkit_frame_get_js_context_for_script_world(frame, world);
        JSCValue *js_global_object = jsc_context_get_global_object(js_context);
        printf("window_object_cleared_callback: page %ld created for %s\n", webkit_web_page_get_id(web_page),
               webkit_web_page_get_uri(web_page));
        // Now we add the Csound API to the context.
        printf("window_object_cleared_callback: defining gjs_csound_hello...\n");
        JSCValue *hello = jsc_value_new_function(js_context,
                          "gjs_csound_hello",
                          G_CALLBACK(on_gjs_csound_hello),
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          0);
        printf("window_object_cleared_callback: defined gjs_csound_hello: %p\n%s\n", hello, jsc_value_to_json(hello, 4));
        printf("web_page_created_callback: defined gjs_csound_hello: %p\n%s\n", hello, jsc_value_to_json(hello, 4));
        auto result = jsc_value_function_call(hello, G_TYPE_NONE);
        jsc_context_set_value (js_context,
                       "hello",
                       hello);    
        }

    void SILENCE_PUBLIC 
    web_page_created_callback(WebKitWebExtension *extension,
            WebKitWebPage *web_page,
            const GVariant *user_data)
    {
        printf("web_page_created_callback: page %ld created for %s\n", webkit_web_page_get_id(web_page),
               webkit_web_page_get_uri(web_page));
        WebKitFrame *web_kit_frame = webkit_web_page_get_main_frame(web_page);
        JSCContext *js_context = webkit_frame_get_js_context(web_kit_frame);
        JSCValue *js_global_object = jsc_context_get_global_object(js_context);
        printf("web_page_created_callback: js_global_object: %p\n", js_global_object);
        // Now we add the Csound API to the context.
        printf("web_page_created_callback: defining gjs_csound_hello...\n");
        JSCValue *hello = jsc_value_new_function(js_context,
                          "gjs_csound_hello",
                          G_CALLBACK(on_gjs_csound_hello),
                          NULL,
                          NULL,
                          G_TYPE_NONE,
                          0);
        printf("web_page_created_callback: defined gjs_csound_hello: %p\n%s\n", hello, jsc_value_to_json(hello, 4));
        auto result = jsc_value_function_call(hello, G_TYPE_NONE);
    }

    /**
     * Loads this extension when the page is created.
     */
    void SILENCE_PUBLIC 
    webkit_web_extension_initialize_with_user_data(WebKitWebExtension *extension,
            GVariant *user_data)
    {
        printf("webkit_web_extension_initialize: %p user_data: %ld\n", extension, g_variant_get_int64(user_data));
        g_signal_connect (extension, "page-created",
                          G_CALLBACK (web_page_created_callback),
                          user_data);
        g_signal_connect (webkit_script_world_get_default (),
                          "window-object-cleared",
                          G_CALLBACK (window_object_cleared_callback),
                          user_data);
    }
};
