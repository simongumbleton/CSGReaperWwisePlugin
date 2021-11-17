###### [Home](../../index.md)
## Create Window

<img src="../../docs/images/PluginWindowCreate.png" width="400">

This window allows for quick creation of several Wwise structural objects.
It uses Waapi to communicate with the Wwise authoring tool, so depends on having the Wwise project open and Waapi configured.  

The title summarises the basic idea of this plugin window.  
<img src="../../docs/images/createtitle.png" width="600"> <br />

### The basic workflow for using the plugin is;
<details>
  <summary> Workflow </summary>
<br />
  1. In Wwise, select the existing object you want to be the parent of your new objects <br />
  <img src="../../docs/images/createselectedparent.png" width="400"> <br />
  <br />
  2. Choose the type of object you want to create, and fill out any additional properties<br />
  You can specify a name and notes, as well as choose what to do if there is a name conflict with an    existing wwise object. You can also create an event for your new object.
  <img src="../../docs/images/createobject.png" width="400"> <br />
  <br />
  3. Press the Create Wwise Object button to create a new object, using the properties you defined, underneath the currently selected Wwise object <br />
</details>

****

### Creating Multiple Objects At Once;
<details>
  <summary> Creating Multiple Objects </summary>
    <br />
  -<strong> The Count property allows you to create multiple objects in one go </strong> <br />
  Setting the count to something greater than 1 will trigger this behaviour.<br />
  <img src="../../docs/images/createmultiplugin.png" width="400"> <br />
  <br />
  -<strong> When this mode is active, the name conflict behaviour is changed to Rename</strong> <br />
  This allows Wwise to handle the incrementing of the object names.<br />
  <img src="../../docs/images/createmultiwwise.png" width="400"> <br />
  <br />
</details>

****

### Wwise Connection and Status;
<details>
  <summary> Wwise Connection and Status </summary>
  <br />
  -<strong> When the plugin launches, it tries to connect to a Wwise project via Waapi. You must ensure that Waapi is enabled in the Wwise authoring tool </strong> <br />
  Take note of the WAMP port you are using. 8080 is the default for the plugin, but it can be changed in the plugin config file <br />
  <img src="../../docs/images/wwisewaapisetup.png" width="400"> <br />
  <br />
  -<strong> If the plugin has a connection, it will display the status and name of the Wwise project at the bottom</strong> <br />
  If the connection is lost, the Connect To Wwise button allows you to try and re-establish a connection<br />
  <img src="../../docs/images/wwisestatusok.png" width="800"> <br />
  <br />
    -<strong> If the plugin cannot make a connection, it will display a no connection warning</strong> <br />
  The most likely cause of this is a mismatch in the ports being used by Wwise and the plugin<br />
  <img src="../../docs/images/statusnowwise.png" width="800"> <br />
  <br />
</details>

****
