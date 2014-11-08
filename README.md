Cash-Register
=============

Implementation of a cash register using Berkeley Sockets

An application involving cash registers implemented using a client-sever architecture communicating over a network. The client provides a sequence of codes for various products on sale along with desired quantity for each. The server returns the prices for each product and maintains a running total of the purchase. Upon closing of the session the sever returns the total cost to client.

The database of the products is maintained in an external text file.
