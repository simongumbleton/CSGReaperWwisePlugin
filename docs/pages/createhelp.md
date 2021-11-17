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
