using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace NickvisionSpotlight.UI
{
    /// <summary>
    /// A class for sending messages (triggering actions) between Views.
    /// </summary>
    public class Messenger
    {
        private static Messenger? _instance;

        public static Messenger Current => _instance ??= new Messenger();

        private readonly Dictionary<string, Action<object?>> _syncCallbacks;
        private readonly Dictionary<string, Func<object?, Task>> _asyncCallbacks;
        private readonly Dictionary<string, bool> _asyncLookup;

        /// <summary>
        /// Creates a Messenger.
        /// </summary>
        private Messenger()
        {
            _syncCallbacks = new Dictionary<string, Action<object?>>();
            _asyncCallbacks = new Dictionary<string, Func<object?, Task>>();
            _asyncLookup = new Dictionary<string, bool>();
        }

        /// <summary>
        /// Registers a new message with a synchronous callback.
        /// </summary>
        /// <param name="messageName">The identifying name of the message</param>
        /// <param name="syncCallback">The synchronous callback function</param>
        /// <exception cref="ArgumentException">Thrown if a message with the provided name has already been registered</exception>
        public void Register(string messageName, Action<object?> syncCallback)
        {
            if (_asyncLookup.ContainsKey(messageName))
            {
                throw new ArgumentException("A message with this name is already registered.");
            }
            _syncCallbacks.Add(messageName, syncCallback);
            _asyncLookup.Add(messageName, false);
        }

        /// <summary>
        /// Registers a new message with an asynchronous callback.
        /// </summary>
        /// <param name="messageName">The identifying name of the message</param>
        /// <param name="asyncCallback">The asynchronous callback function</param>
        /// <exception cref="ArgumentException">Thrown if a message with the provided name has already been registered</exception>
        public void Register(string messageName, Func<object?, Task> asyncCallback)
        {
            if (_asyncLookup.ContainsKey(messageName))
            {
                throw new ArgumentException("A message with this name is already registered.");
            }
            _asyncCallbacks.Add(messageName, asyncCallback);
            _asyncLookup.Add(messageName, true);
        }

        /// <summary>
        /// Deregisters a message.
        /// </summary>
        /// <param name="messageName">The identifying name of the message</param>
        /// <exception cref="ArgumentException">Thrown if a message with the provided name has not been registered</exception>
        public void Deregister(string messageName)
        {
            if (!_asyncLookup.ContainsKey(messageName))
            {
                throw new ArgumentException("A message with this name is not registered.");
            }
            if (_asyncLookup[messageName])
            {
                _asyncCallbacks.Remove(messageName);
            }
            else
            {
                _syncCallbacks.Remove(messageName);
            }
            _asyncLookup.Remove(messageName);
        }

        /// <summary>
        /// Sends a message (triggering the callback function associated with that message).
        /// </summary>
        /// <param name="messageName">The identifying name of the message</param>
        /// <param name="parameter">A parameter to pass to a message's callback</param>
        public async void Send(string messageName, object? parameter = null)
        {
            if (!_asyncLookup.ContainsKey(messageName))
            {
                return;
            }
            if (_asyncLookup[messageName])
            {
                await _asyncCallbacks[messageName](parameter);
            }
            else
            {
                _syncCallbacks[messageName](parameter);
            }
        }

        /// <summary>
        /// Sends a message (triggering the callback function associated with that message).
        /// </summary>
        /// <param name="messageName">The identifying name of the message</param>
        /// <param name="parameter">A parameter to pass to a message's callback</param>
        /// <returns>Returns true if the message was received (the callback was executed), else false</returns>
        public async Task<bool> SendAsync(string messageName, object? parameter = null)
        {
            if (!_asyncLookup.ContainsKey(messageName))
            {
                return false;
            }
            if (_asyncLookup[messageName])
            {
                await _asyncCallbacks[messageName](parameter);
            }
            else
            {
                _syncCallbacks[messageName](parameter);
            }
            return true;
        }
    }
}