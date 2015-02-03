## Synopsis

This library is part of the Manus SDK and provides functionality to communicate with the Manus Glove and the Manus Interface. Currently only communication with the Manus Glove is implemented.

## Usage

To communicate with the Manus Glove the SDK has to be initialized with ManusInit() after which the current state of a glove can be retrieved with ManusGetState().

When no longer using the SDK ManusExit() should be called so that the SDK can safely shut down.

## Code Example

A minimal program to retrieve the current state and the yaw, pitch and roll of the first connected Manus Glove looks like this:

	ManusInit();
	
	GLOVE_STATE state;
	while (true)
	{
		if (ManusGetState(0, &state) == MANUS_SUCCESS)
		{
			GLOVE_VECTOR euler, gravity;
			ManusGetGravity(&gravity, &state.data.Quaternion);
			ManusGetEuler(&euler, &state.data.Quaternion, &gravity);
		}
		else
		{
			// The requested glove is likely not connected
		}
	}

Other gloves can also be queried and the number of currently connected gloves can be retrieved with ManusGetGloveCount().

## Documentation

The full documentation is available at [labs.manusmachina.com](http://labs.manusmachina.com/).

## License

The Manus SDK is licensed under LGPL and the Manus Interface is licensed under GPL.
