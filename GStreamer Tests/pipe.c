#include <gst/gst.h>
#include <glib.h>

GstElement *pipeline, *effect, *effect, *conv1, *conv2, *source, *sink;

void start_pipeline(int effect_num)
{
	GMainLoop *loop;
	/* Initialization */
	gst_init (0, NULL);
	loop = g_main_loop_new (NULL, FALSE);


	/* Create constant gstreamer elements (we wont change these ever)*/
	printf("[pipe]:\tCreating pipeline elements\n");
	pipeline = gst_pipeline_new ("filter-pipeline");
	source = gst_element_factory_make ("alsasrc", "audio-source");
	conv1 = gst_element_factory_make ("audioconvert", "converter src");
	conv2 = gst_element_factory_make ("audioconvert", "converter sink");
	sink = gst_element_factory_make ("alsasink", "audio-sink");

	if (!pipeline || !source || !conv1 || !conv2 || !sink) {
		g_printerr ("One element could not be created. Exiting.\n");
		return;
	}

	/* Create effects elements */
	printf("[pipe]:\tSetting up default filter parameters\n");
	switch( effect_num )
	{
		case 0:
			effect = gst_element_factory_make ("audiocheblimit", "cheb-LPF");
			g_object_set (G_OBJECT (effect), "mode", 0, NULL);
			g_object_set (G_OBJECT (effect), "cutoff", (float)1000, NULL);
			break;
		case 1:
			effect = gst_element_factory_make ("audiochebband", "cheb-BPF");
			g_object_set (G_OBJECT (effect), "upper-frequency", (float)1050, NULL);
			g_object_set (G_OBJECT (effect), "lower-frequency", (float)950, NULL);
			break;
	}

	/* audio-source -> converter -> cheb filter -> converter -> alsa-output */
	printf("[pipe]:\tBuilding the pipeline\n");
	gst_bin_add_many (GST_BIN (pipeline),
	source, conv1, effect, conv2, sink, NULL);
	gst_element_link_many (source, conv1, effect, conv2, sink, NULL);

	printf("[pipe]:\tNow playing\n");
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
}

void stop_pipeline(){
	printf("[pipe]:\tStopping playback\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));
}

void restart_pipeline(int effect_num){
	stop_pipeline();
	start_pipeline(effect_num);
}


void configure_LPF(float cutoff){
	printf("[pipe]:\tSetting LP cutoff to %f\n", cutoff);
	g_object_set (G_OBJECT (effect), "cutoff", cutoff, NULL);
}

void configure_BPF(float center, float bandwidth){
	float upper, lower;

	upper = center+bandwidth/2;
	if( upper < 0 )
		upper = 0;
	else if( upper > 100000 )
		upper = 100000;

	lower = center-bandwidth/2;
	if( lower < 0 )
		lower = 0; 
	else if( lower > 100000 )
		 lower = 100000;

	printf("[pipe]:\tSetting BP upper to %f, lower to %f\n", upper, lower);
	g_object_set (G_OBJECT (effect), "upper-frequency", upper, NULL);
	g_object_set (G_OBJECT (effect), "lower-frequency", lower, NULL);
}
