#include <gst/gst.h>
#include <glib.h>

GstElement *pipeline, *effect, *effect, *conv1, *conv2, *source, *sink;

void start_pipeline(int effect_num)
{
	GMainLoop *loop;
	/* Initialisation */
	gst_init (0, NULL);
	loop = g_main_loop_new (NULL, FALSE);

	/* Create constant gstreamer elements (we wont change these ever)*/
	pipeline = gst_pipeline_new ("filter-pipeline");
	source = gst_element_factory_make ("alsasrc", "audio-source");
	conv1 = gst_element_factory_make ("audioconvert", "converter src");
	conv2 = gst_element_factory_make ("audioconvert", "converter sink");
	sink = gst_element_factory_make ("alsasink", "audio-sink");

	/* Create effects elements */

	if (!pipeline || !source || !conv1 || !conv2 || !sink) {
		g_printerr ("One element could not be created. Exiting.\n");
		return;
	}

	switch( effect_num )
	{
		case 0:
			effect = gst_element_factory_make ("audiocheblimit", "cheb-LPF");
			g_print("Setting up filter");
			g_object_set (G_OBJECT (effect), "mode", 0, NULL);
			g_object_set (G_OBJECT (effect), "cutoff", (float)1000, NULL);
			break;
		case 1:
			effect = gst_element_factory_make ("audiochebband", "cheb-BPF");
			g_print("Setting up filter");
			g_object_set (G_OBJECT (effect), "upper-frequency", (float)1050, NULL);
			g_object_set (G_OBJECT (effect), "lower-frequency", (float)950, NULL);
			break;
	}

	/* we add all elements into the pipeline */
	/* audio-source | converter | cheb filter | converter | alsa-output */
	g_print("Building the pipeline\n");
	gst_bin_add_many (GST_BIN (pipeline),
	source, conv1, effect, conv2, sink, NULL);
	/* we link the elements together */
	/* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
	gst_element_link_many (source, conv1, effect, conv2, sink, NULL);

	g_print ("Now playing\n");
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
}

void stop_pipeline(){
	g_print ("[pipe]:\tStopping playback\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
	g_print ("[pipe]:\tDeleting pipeline\n");	
	gst_object_unref (GST_OBJECT (pipeline));
	/*gst_object_unref (GST_OBJECT (source));
	gst_object_unref (GST_OBJECT (conv1));
	gst_object_unref (GST_OBJECT (effect));
	gst_object_unref (GST_OBJECT (conv2));
	gst_object_unref (GST_OBJECT (sink));*/
}

void restart_pipeline(int effect_num){
	stop_pipeline();
	start_pipeline(effect_num);
}


void configure_LPF(float cutoff){
	g_object_set (G_OBJECT (effect), "cutoff", cutoff, NULL);
}

void configure_BPF(float center, float bandwidth){
	float upper, lower;

	upper = center-bandwidth/2;
	if( upper < 0 )
		upper = 0;
	else if( upper > 100000 )
		upper = 100000;

	lower = center-bandwidth/2;
	if( lower < 0 )
		lower = 0; 
	else if( lower > 100000 )
		 lower = 100000;

	g_object_set (G_OBJECT (effect), "upper-frequency", upper, NULL);
	g_object_set (G_OBJECT (effect), "lower-frequency", lower, NULL);
}

/*
void set_effect(int effect_num){
	GstPad *conv_src;
	GstElement *effect;
	
	switch( effect_num )
	{
		case 0:
			effect = effect_LPF;
			break;
		case 1:
			effect = effect_BPF;
			break;
	}

	printf("[pipe]:\tPausing pipeline\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
*/

	/*
	conv_src = gst_element_get_pad(conv1, "src");

	// Make sure no data is being sourced
	printf("[pipe]:\tBlocking pad\n");
	gst_pad_set_blocked(conv_src, 1);
	*/

	// Unlink old effect
/*
	printf("[pipe]:\tUnlinking Previous Element\n");
	gst_object_unref (GST_OBJECT (pipeline));
*/
	/*
	gst_element_unlink(conv1, effect_CURR); 
	gst_element_unlink(effect_CURR, conv2);
	*/
/*
	effect_CURR = effect;
	pipeline = gst_pipeline_new ("filter-pipeline");
	source = gst_element_factory_make ("alsasrc", "audio-source");
	conv1 = gst_element_factory_make ("audioconvert", "converter src");
	conv2 = gst_element_factory_make ("audioconvert", "converter sink");
	sink = gst_element_factory_make ("alsasink", "audio-sink");
*/

	/* Create effects elements */
/*
	effect_LPF = gst_element_factory_make ("audiocheblimit", "cheb-LPF");
	effect_BPF = gst_element_factory_make ("audiochebband", "cheb-BPF");

	if (!pipeline || !source || !conv1 || !effect_LPF || !effect_BPF || !conv2 || !sink) {
		g_printerr ("One element could not be created. Exiting.\n");
		return -1;
	}
	
	effect_CURR=effect_BPF;
*/

	/* Set up initial filter configurations */
/*
	g_print("Setting up filters");
	g_object_set (G_OBJECT (effect_LPF), "mode", 0, NULL);
	g_object_set (G_OBJECT (effect_LPF), "cutoff", (float)1000, NULL);

	g_object_set (G_OBJECT (effect_BPF), "upper-frequency", (float)1050, NULL);
	g_object_set (G_OBJECT (effect_BPF), "lower-frequency", (float)950, NULL);
	gst_bin_add_many (GST_BIN (pipeline),
	source, conv1, effect_LPF, effect_BPF, conv2, sink, NULL);

	// Link new effect
	printf("[pipe]:\tLinking New Element\n");
	if(!gst_element_link_many (source, conv1, effect_CURR, conv2, sink, NULL))
		printf("[pipe]:\tFailed to link new element\n");
*/

	/*
	// Allow data to be sourced again
	printf("[pipe]:\tUnblocking pad\n");
	gst_pad_set_blocked(conv_src, 0);
	*/

/*
	printf("[pipe]:\tPlaying pipeline\n");
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
}
*/
