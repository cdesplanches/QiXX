import { describe, it, expect, beforeAll, afterAll } from "vitest";
import { uePost, createTestBlueprint, deleteTestBlueprint, uniqueName } from "../helpers.js";

/**
 * Tests for mutation tools that operate on variables, function parameters,
 * and struct node types. These tools require pre-existing variables/functions
 * that can't be created via the current API, so we primarily test error cases
 * and verify the response structure.
 */

describe("replace_function_calls", () => {
  const bpName = uniqueName("BP_ReplaceFnTest");
  const packagePath = "/Game/Test";

  beforeAll(async () => {
    const bp = await createTestBlueprint({ name: bpName });
    expect(bp.error).toBeUndefined();
  });

  afterAll(async () => {
    await deleteTestBlueprint(`${packagePath}/${bpName}`);
  });

  it("returns error when newClass does not exist", async () => {
    // The API validates newClass exists before scanning — returns error if not found
    const data = await uePost("/api/replace-function-calls", {
      blueprint: bpName,
      oldClass: "FakeOldLibrary_XYZ",
      newClass: "FakeNewLibrary_XYZ",
    });
    expect(data.error).toBeDefined();
    expect(data.error).toContain("FakeNewLibrary_XYZ");
  });

  it("returns error in dryRun mode when newClass does not exist", async () => {
    const data = await uePost("/api/replace-function-calls", {
      blueprint: bpName,
      oldClass: "FakeOldLibrary_XYZ",
      newClass: "FakeNewLibrary_XYZ",
      dryRun: true,
    });
    expect(data.error).toBeDefined();
  });

  it("returns error for non-existent blueprint", async () => {
    const data = await uePost("/api/replace-function-calls", {
      blueprint: "BP_DoesNotExist_XYZ_999",
      oldClass: "SomeClass",
      newClass: "OtherClass",
    });
    expect(data.error).toBeDefined();
  });

  it("rejects missing required fields", async () => {
    const data = await uePost("/api/replace-function-calls", {
      blueprint: bpName,
    });
    expect(data.error).toBeDefined();
  });
});

describe("change_variable_type", () => {
  const bpName = uniqueName("BP_ChangeVarTest");
  const packagePath = "/Game/Test";

  beforeAll(async () => {
    const bp = await createTestBlueprint({ name: bpName });
    expect(bp.error).toBeUndefined();
  });

  afterAll(async () => {
    await deleteTestBlueprint(`${packagePath}/${bpName}`);
  });

  it("returns error for non-existent variable", async () => {
    const data = await uePost("/api/change-variable-type", {
      blueprint: bpName,
      variable: "NonExistentVar_XYZ",
      newType: "FVector",
      typeCategory: "struct",
    });
    expect(data.error).toBeDefined();
  });

  it("returns error for non-existent blueprint", async () => {
    const data = await uePost("/api/change-variable-type", {
      blueprint: "BP_DoesNotExist_XYZ_999",
      variable: "SomeVar",
      newType: "FVector",
      typeCategory: "struct",
    });
    expect(data.error).toBeDefined();
  });

  it("rejects missing required fields", async () => {
    const data = await uePost("/api/change-variable-type", {
      blueprint: bpName,
    });
    expect(data.error).toBeDefined();
  });
});

describe("change_function_parameter_type", () => {
  const bpName = uniqueName("BP_ChangeFnParamTest");
  const packagePath = "/Game/Test";

  beforeAll(async () => {
    const bp = await createTestBlueprint({ name: bpName });
    expect(bp.error).toBeUndefined();
  });

  afterAll(async () => {
    await deleteTestBlueprint(`${packagePath}/${bpName}`);
  });

  it("returns error for non-existent function", async () => {
    const data = await uePost("/api/change-function-param-type", {
      blueprint: bpName,
      functionName: "NonExistentFunc_XYZ",
      paramName: "SomeParam",
      newType: "FVector",
    });
    expect(data.error).toBeDefined();
    expect(data.availableFunctionsAndEvents).toBeDefined();
  });

  it("returns error for non-existent blueprint", async () => {
    const data = await uePost("/api/change-function-param-type", {
      blueprint: "BP_DoesNotExist_XYZ_999",
      functionName: "SomeFunc",
      paramName: "SomeParam",
      newType: "FVector",
    });
    expect(data.error).toBeDefined();
  });

  it("rejects missing required fields", async () => {
    const data = await uePost("/api/change-function-param-type", {
      blueprint: bpName,
    });
    expect(data.error).toBeDefined();
  });
});

describe("remove_function_parameter", () => {
  const bpName = uniqueName("BP_RemoveParamTest");
  const packagePath = "/Game/Test";

  beforeAll(async () => {
    const bp = await createTestBlueprint({ name: bpName });
    expect(bp.error).toBeUndefined();
  });

  afterAll(async () => {
    await deleteTestBlueprint(`${packagePath}/${bpName}`);
  });

  it("returns error for non-existent function", async () => {
    const data = await uePost("/api/remove-function-parameter", {
      blueprint: bpName,
      functionName: "NonExistentFunc_XYZ",
      paramName: "SomeParam",
    });
    expect(data.error).toBeDefined();
  });

  it("returns error for non-existent blueprint", async () => {
    const data = await uePost("/api/remove-function-parameter", {
      blueprint: "BP_DoesNotExist_XYZ_999",
      functionName: "SomeFunc",
      paramName: "SomeParam",
    });
    expect(data.error).toBeDefined();
  });

  it("rejects missing required fields", async () => {
    const data = await uePost("/api/remove-function-parameter", {
      blueprint: bpName,
    });
    expect(data.error).toBeDefined();
  });
});

describe("change_struct_node_type", () => {
  const bpName = uniqueName("BP_ChangeStructTest");
  const packagePath = "/Game/Test";

  beforeAll(async () => {
    const bp = await createTestBlueprint({ name: bpName });
    expect(bp.error).toBeUndefined();
  });

  afterAll(async () => {
    await deleteTestBlueprint(`${packagePath}/${bpName}`);
  });

  it("returns error for non-existent node", async () => {
    const data = await uePost("/api/change-struct-node-type", {
      blueprint: bpName,
      nodeId: "00000000-0000-0000-0000-000000000000",
      newType: "FVector",
    });
    expect(data.error).toBeDefined();
  });

  it("returns error for non-existent blueprint", async () => {
    const data = await uePost("/api/change-struct-node-type", {
      blueprint: "BP_DoesNotExist_XYZ_999",
      nodeId: "some-node-id",
      newType: "FVector",
    });
    expect(data.error).toBeDefined();
  });

  it("rejects missing required fields", async () => {
    const data = await uePost("/api/change-struct-node-type", {
      blueprint: bpName,
    });
    expect(data.error).toBeDefined();
  });
});
