## Synopsis

This library is part of the Manus SDK and provides functionality to communicate with the Manus Glove and the Manus Interface. Currently only communication with the Manus Glove is implemented.

## Usage

To communicate with the Manus Glove the SDK has to be initialized with ManusInit() after which the current state of a glove can be retrieved with ManusGetData().

When no longer using the SDK ManusExit() should be called so that the SDK can safely shut down.

## Code Example

A minimal program to retrieve the data from the left Manus Glove looks like this:

	ManusInit();
	
	GLOVE_DATA data;
	while (true)
	{
		if (ManusGetData(GLOVE_LEFT, &state) == MANUS_SUCCESS)
		{
			// The data structure now contains the glove data
		}
		else
		{
			// The requested glove is not connected
		}
	}
	
	ManusExit();

Other gloves can also be queried and the number of currently connected gloves can be retrieved with ManusGetGloveCount().

## Documentation

The full documentation is available at [labs.manusmachina.com](http://labs.manusmachina.com/).

## License

The Manus SDK is licensed under LGPL and the Manus Interface is licensed under GPL.
