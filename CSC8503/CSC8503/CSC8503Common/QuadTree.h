#pragma once
//#include "../../Common/Vector2.h"
#include "Debug.h"
#include <list>
#include <functional>
#include "CollisionDetection.h"

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry
		{
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size)
			{
				object = obj;
				this->pos = pos;
				this->size = size;
			}
		};


		template<class T>
		class QuadTreeNode	{
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() /*: parentTree(nullptr)*/ {
				children = nullptr;
			}

			QuadTreeNode(/*QuadTree<T>&parent,*/ Vector2 pos, Vector2 size)/* : parentTree(parent)*/ {
				children		= nullptr;
				this->position	= pos;
				this->size		= size;
			}

			~QuadTreeNode() {
				delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
				if(!CollisionDetection::AABBTest(objectPos, Vector3(position.x,0,position.y), objectSize, Vector3(size.x, 1000.0f, size.y)))
					return;

				if (children) //Not a leaf node then go further into the tree
					for (int i = 0; i < 4; ++i)
						children[i].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize);
				else //is a leaf node
				{
					contents.push_back(QuadTreeEntry<T>(object, objectPos, objectSize));
					if ((int)contents.size() > maxSize && depthLeft > 0)
					{
						if(!children)
						{
							Split();
							//Need to reinsert contents
							for(const auto& i : contents)
							{
								for(int j = 0; j < 4; ++j)
								{
									auto entry = i;
									children[j].Insert(entry.object, entry.pos, entry.size, depthLeft - 1, maxSize);
								}
							}
							contents.clear();
						}
					}
				}
			}

			void Split() {
				//Split this quad tree node into 4
				Vector2 halfSize = size / 2.0f;
				children = new QuadTreeNode<T>[4];
				children[0] = QuadTreeNode<T>(position + Vector2(-halfSize.x, halfSize.y), halfSize);
				children[1] = QuadTreeNode<T>(position + Vector2(halfSize.x, halfSize.y), halfSize);
				children[2] = QuadTreeNode<T>(position + Vector2(-halfSize.x, -halfSize.y), halfSize);
				children[3] = QuadTreeNode<T>(position + Vector2(halfSize.x, -halfSize.y), halfSize);
			}

			void DebugDraw() {
				Vector4 colour = Vector4(1, 0, 0, 1);
				Vector3 corner1 = Vector3(position.x, 0, position.y) + Vector3(size.x, 0, size.y);
				Vector3 corner2 = Vector3(position.x, 0, position.y) + Vector3(size.x, 0, -size.y);
				Vector3 corner3 = Vector3(position.x, 0, position.y) + Vector3(-size.x, 0, -size.y);
				Vector3 corner4 = Vector3(position.x, 0, position.y) + Vector3(-size.x, 0, size.y);

				Debug::DrawLine(corner1, corner2, colour);
				Debug::DrawLine(corner2, corner3, colour);
				Debug::DrawLine(corner3, corner4, colour);
				Debug::DrawLine(corner4, corner1, colour);

				if(children)
				{
					for (int i = 0; i < 4; ++i)
					{
						children[i].DebugDraw();
					}
				}

			}

			void OperateOnContents(QuadTreeFunc& func) {
				if (children)
				{
					for (int i = 0; i < 4; ++i)
					{
						children[i].OperateOnContents(func);
					}
				}
				else
				{
					if(!contents.empty())
					{
						func(contents);
					}
				}
			}

		protected:
			std::list<QuadTreeEntry<T>>	contents;
			//QuadTree<T>&	parentTree;

			Vector2 position;
			Vector2 size;

			QuadTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree	{
		public:
			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5) 
			: root(QuadTreeNode<T>(/**this, */Vector2(), size)){
				this->maxDepth	= maxDepth;
				this->maxSize	= maxSize;
			}
			~QuadTree() {
			}

			void Insert(T object, const Vector3& pos, const Vector3& size) {
				root.Insert(object, pos, size, maxDepth, maxSize);
			}

			void DebugDraw() {
				root.DebugDraw();
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root.OperateOnContents(func);
			}

		protected:
			QuadTreeNode<T> root;
			int maxDepth;
			int maxSize;
		};
	}
}