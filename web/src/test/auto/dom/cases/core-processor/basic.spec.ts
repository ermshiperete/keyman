import { assert } from 'chai';
import { CoreKeyboard, CoreProcessor, KM_CORE_STATUS } from 'keyman/engine/core-processor';

const coreurl = '/web/build/engine/core-processor/obj/import/core';

// Test the CoreProcessor interface.
describe('CoreProcessor', function () {
  it('can initialize without errors', async function () {
    const kp = new CoreProcessor();
    assert.isTrue(await kp.init(coreurl));
  });

  it('can call temp function', async function () {
    const kp = new CoreProcessor();
    await kp.init(coreurl);
    const a = kp.tmp_wasm_attributes();
    assert.isNotNull(a);
    assert.isNumber(a.max_context);
    console.dir(a);
  });

  it('can load a keyboard from blob', async function () {
    const kp = new CoreProcessor();
    await kp.init(coreurl);
    let kb: CoreKeyboard = {};
    const status = kp.km_core_keyboard_load_from_blob('test', new Uint8Array([0x4b, 0x58, 0x54, 0x53]), kb);
    assert.equal(status, KM_CORE_STATUS.OK);
    assert.isNotNull(kb);
  });
});
