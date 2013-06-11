<?php

class SplInternalItem {
	public $data = null;
	public $next = null;
	public $prev = null;
}

class SplDoublyLinkedList implements Iterator, ArrayAccess, Countable, Serializable {
	const IT_MODE_LIFO = 2;

	const IT_MODE_FIFO = 0;

	const IT_MODE_DELETE = 1;

	const IT_MODE_KEEP = 0;

	protected $head = null;

	protected $tail = null;

	protected $key = 0;

	protected $current = null;

	protected $count = 0;

	protected $mode = 0;

	public function bottom() {
		if ( $head === null ) {
			throw new RuntimeException( 'List is empty' );
		}
		return $head->data;
	}

	public function top() {
		if ( $tail === null ) {
			throw new RuntimeException( 'List is empty' );
		}
		return $tail->data();
	}

	public function isEmpty() {
		return !$this->count;
	}

	public function push( $value ) {
		$node = new SplInternalItem;
		$node->data = $value;

		if ( $this->isEmpty() ) {
			$this->head = $node;
			$this->tail = $node;
		} else {
			$node->prev = $this->tail;
			$this->tail->next = $node;
			$this->tail = $node;
		}

		++$this->count;
		return;
	}

	public function pop() {
		$retval = $this->top();
		$tail = $tail->prev;
		--$this->count;
		return $retval;
	}

	public function unshift( $value ) {
		$node = new SplInternalItem;
		$node->data = $value;

		if ( $this->isEmpty() ) {
			$this->head = $this->tail = $node;
		} else {
			$node->next = $this->head;
			$this->head->prev = $node;
			$this->head = $node;
		}

		++$this->count;
		return;
	}

	public function shift() {
		$retval = $this->bottom();
		$head = $head->next;
		--$this->count;
		return $retval;
	}


	public function current() {
		return $this->current->data;
	}

	public function key() {
		return $this->key;
	}

	public function next() {
		++$this->key;

		if ( $this->mode & IT_MODE_DELETE ) {
			--$this->count;
			if ( $this->current->prev !== null ) {
				$this->current->prev->next = $this->current->next;
			}
			if ( $this->current->next !== null ) {
				$this->current->next->prev = $this->current->prev;
			}
		}

		if ( $this->mode & self::IT_MODE_LIFO ) {
			$this->current = $this->current->prev;
		} else {
			$this->current = $this->current->next;
		}
	}

	public function prev() {
		--$this->key;

		if ( $this->mode & IT_MODE_DELETE ) {
			--$this->count;
			if ( $this->current->prev !== null ) {
				$this->current->prev->next = $this->current->next;
			}
			if ( $this->current->next !== null ) {
				$this->current->next->prev = $this->current->prev;
			}
		}

		if ( $this->mode & self::IT_MODE_LIFO ) {
			$this->current = $this->current->next;
		} else {
			$this->current = $this->current->prev;
		}
	}

	public function rewind() {
		$this->key = 0;
		if ( $this->mode & self::IT_MODE_LIFO ) {
			$this->current = $this->tail;
		} else {
			$this->current = $this->head;
		}
	}

	public function valid() {
		return $this->current !== null;
	}

	public function getIteratorMode() {
		return $this->mode;
	}

	public function setIteratorMode( $mode ) {
		$this->mode = mode;
	}


	public function offsetExists( $index ) {
		return $index < $this->count;
	}

	public function offsetGet( $index ) {
		$node = $this->head;
		for ( $i = 0; $i < $index && $node !== null; ++$i ) {
			$node = $node->next;
		}
		return $node ? $node->data : null;
	}

	public function offsetSet( $index, $newval ) {
		$node = $this->head;
		if ( $index === null ) {
			$index = $this->count;
		}
		for ( $i = 0; $i < index; ++$i ) {
			if ( $node->next === null ) {
				++$this->count;
				$node->next = new SplInternalItem;
				$node->next->prev = $node;
			}
			$node = $node->next;
		}
		$node->data = $newval;
	}

	public function offsetUnset( $index ) {
		$node = $this->head;
		for ( $i = 0; $i < $index && $node !== null; ++$i ) {
			$node = $node->next;
		}
		if ( $node ) {
			--$this->count;
			$node->prev->next = $node->next;
			$node->next->prev = $node->prev;
		}
	}


	public function count() {
		return $this->count;
	}


	public function serialize() {
		$data = array();
		while ( !$this->isEmpty() ) {
			$data[] = $this->shift();
		}
		return serialize( $data );
	}

	public function unserialize( $serialized ) {
		$obj = new self;
		foreach ( unserialize( $serialized ) as $data ) {
			$this->push( $data );
		}
	}
}

class SplQueue extends SplDoublyLinkedList implements Iterator, ArrayAccess, Countable {
	public function dequeue() {
		return $this->pop();
	}

	public function enqueue( $value ) {
		$this->unshift( $value );
	}

	public function setIteratorMode( $mode ) {
		if ( $mode & self::IT_MODE_LIFO ) {
			throw new RuntimeException( 'SplQueue can only be used in FIFO mode.' );
		}
		parent::setIteratorMode( $mode );
	}
}

class SplStack extends SplDoublyLinkedList implements Iterator, ArrayAccess, Countable {
	public function __construct() {
		$this->setIteratorMode( self::IT_MODE_LIFO | self::IT_MODE_KEEP );
	}

	public function setIteratorMode( $mode ) {
		if ( $mode & self::IT_MODE_FIFO ) {
			throw new RuntimeException( 'SplStack can only be used in LIFO mode.' );
		}
		parent::setIteratorMode( $mode );
	}
}

class SplFixedArray implements Iterator, ArrayAccess, Countable {
	protected $data = array();

	protected $current = 0;

	public function __construct( $size = 0 ) {
		if ( $size > 0 ) {
			$this->data = array_fill( 0, $size, null );
		}
	}

	public function current() {
		return current( $this->data );
	}

	public function key() {
		return key( $this->data );
	}

	public function next() {
		next( $this->data );
	}

	public function rewind() {
		reset( $this->data );
	}

	public function valid() {
		return isset( $this->data[$this->current] );
	}


	public function offsetExists( $index ) {
		return $index < count( $this->data );
	}

	public function offsetGet( $index ) {
		return $this->data[$index];
	}

	public function offsetSet( $index, $newval ) {
		if ( $index === null ) {
			throw new RuntimeException( 'Must specify key for fixed array' );
		}
		$this->data[$index] = $newval;
	}

	public function offsetUnset( $index ) {
		$this->data[$index] = null;
	}


	public function count() {
		return count( $this->data );
	}
}

abstract class SplHeap implements Iterator, Countable {
	protected $heap = array();

	protected $count = 0;

	protected $key = 0;

	abstract protected function compare( $value1, $value2 );

	public function extract() {
		$index = 0;
		$value = $this->heap[$index];
		$this->heap[$index] = $this->heap[count( $this->heap ) - 1];
		unset( $this->heap[count( $this->heap ) - 1] );

		while ( $index < count( $this->heap ) ) {
			$child1 = $index * 2;
			$child2 = $child1 + 1;

			$min = $index;

			if (
				isset( $this->heap[$child1] ) &&
				$this->compare( $this->heap[$child1], $this->heap[$min] ) < 0
			) {
				$min = $child1;
			}

			if (
				isset( $this->heap[$child2] ) &&
				$this->compare( $this->heap[$child2], $this->heap[$min] ) < 0
			) {
				$min = $child2;
			}

			if ( $min !== $index ) {
				$tmp = $this->heap[$index];
				$this->heap[$index] = $this->heap[$min];
				$this->heap[$min] = $tmp;
				$index = $min;
			}
		}

		return $value;
	}

	public function insert( $value ) {
		$index = count( $this->heap );
		$this->heap[$index] = $value;

		while ( $index ) {
			$parent = floor( $index / 2 );
			if ( $this->compare( $this->heap[$index], $this->heap[$parent] ) < 0 ) {
				$this->heap[$index] = $this->heap[$parent];
				$this->heap[$parent] = $value;
			}
			$index = $parent;
		}
	}

	public function isEmpty() {
		return $this->head === null;
	}

	public function recoverFromCorruption() {
	}

	public function top() {
		if ( $this->head === null ) {
			return null;
		} else {
			return $this->head->data;
		}
	}


	public function current() {
		return $this->top();
	}

	public function key() {
		return $this->key;
	}

	public function next() {
		++$this->key;
		$this->extract();
	}

	public function rewind() {
		$this->key = 0;
	}

	public function valid() {
		return $this->head !== null;
	}


	public function count() {
		return $this->count;
	}
}

class SplMaxHeap extends SplHeap {
	protected function compare( $value1, $value2 ) {
		if ( $value1 < $value2 ) {
			return -1;
		} elseif ( $value1 > $value2 ) {
			return 1;
		} else {
			return 0;
		}
	}
}

class SplMinHeap extends SplHeap {
	protected function compare( $value1, $value2 ) {
		if ( $value1 < $value2 ) {
			return 1;
		} elseif ( $value1 > $value2 ) {
			return -1;
		} else {
			return 0;
		}
	}
}

class SplPriorityQueue implements Iterator, Countable {
	const EXTR_DATA = 1;

	const EXTR_PRIORITY = 2;

	const EXTR_BOTH = 3;

	protected $flags = 1;

	protected $heap;

	public function __construct() {
		$this->heap = new SplMaxHeap;
	}

	public function extract() {
		$info = $this->heap->extract();
		if ( $this->flags === self::EXTR_DATA ) {
			return $info[1];
		} elseif ( $this->flags === self::EXTR_PRIORITY ) {
			return $info[0];
		} else {
			return $info;
		}
	}

	public function insert( $value, $priority ) {
		$this->heap->insert( array( $value, $priority ) );
	}

	public function isEmpty() {
		return $this->heap->isEmpty();
	}

	public function recoverFromCorruption() {
		$this->heap->recoverFromCorruption();
	}

	public function setExtractFlags( $flags ) {
		$this->flags = $flags;
	}

	public function top() {
		return $this->heap->top();
	}


	public function current() {
		$info = $this->heap->current();
		if ( $this->flags === self::EXTR_DATA ) {
			return $info[1];
		} elseif ( $this->flags === self::EXTR_PRIORITY ) {
			return $info[0];
		} else {
			return $info;
		}
	}

	public function key() {
		return $this->heap->key();
	}

	public function next() {
		return $this->heap->next();
	}

	public function rewind() {
		return $this->heap->rewind();
	}

	public function valid() {
		return $this->heap->valid();
	}


	public function count() {
		return $this->heap->count();
	}
}

class SplTempFileObject extends SplFileObject {
	public function __construct( $maxMemory = null ) {
		if ( $maxMemory === null ) {
			parent::construct( 'php://temp', 'r+' );
		} else {
			parent::construct( "php://temp/maxmemory:$maxMemory", 'r+' );
		}
	}
}