<?php
	// Create a new Doubly Linked List
	$dll = new SplDoublyLinkedList();
	
	// Add some items to the list
	$dll->push(1);
	$dll->push(2);
	$dll->push(3);
	
	try {
		$dll->offsetUnset(3);
	}
	catch (Exception $e) {
		echo $e->getMessage() . "\n";
	}
?>