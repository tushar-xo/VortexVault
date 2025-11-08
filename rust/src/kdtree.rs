use std::collections::BinaryHeap;
use std::cmp::Reverse;
use serde::{Serialize, Deserialize};

#[derive(Clone, Serialize, Deserialize)]
struct KDNode {
    point: Vec<f32>,
    id: usize,
    metadata: String,
    left: Option<Box<KDNode>>,
    right: Option<Box<KDNode>>,
}

pub struct KDTreeVectorDB {
    dimension: usize,
    root: Option<Box<KDNode>>,
    next_id: usize,
    all_vectors: Vec<Vec<f32>>,
}

impl KDTreeVectorDB {
    pub fn new(dim: usize) -> Self {
        KDTreeVectorDB {
            dimension: dim,
            root: None,
            next_id: 0,
            all_vectors: Vec::new(),
        }
    }

    fn distance(a: &Vec<f32>, b: &Vec<f32>) -> f32 {
        a.iter().zip(b.iter()).map(|(x, y)| (x - y).powi(2)).sum::<f32>().sqrt()
    }

    fn build_tree(&self, mut points: Vec<(Vec<f32>, usize, String)>, depth: usize) -> Option<Box<KDNode>> {
        if points.is_empty() {
            return None;
        }

        let axis = depth % self.dimension;
        points.sort_by(|a, b| a.0[axis].partial_cmp(&b.0[axis]).unwrap());

        let median = points.len() / 2;
        let node = Box::new(KDNode {
            point: points[median].0.clone(),
            id: points[median].1,
            metadata: points[median].2.clone(),
            left: self.build_tree(points[..median].to_vec(), depth + 1),
            right: self.build_tree(points[median + 1..].to_vec(), depth + 1),
        });
        Some(node)
    }

    pub fn insert(&mut self, vec: Vec<f32>, meta: String) -> usize {
        if vec.len() != self.dimension {
            panic!("Dimension mismatch");
        }
        let id = self.next_id;
        self.next_id += 1;
        self.all_vectors.push(vec.clone());
        if self.root.is_none() {
            self.root = Some(Box::new(KDNode {
                point: vec,
                id,
                metadata: meta,
                left: None,
                right: None,
            }));
        } else {
            self.insert_rec(self.root.as_mut().unwrap(), vec, id, meta, 0);
        }
        id
    }

    fn insert_rec(&self, node: &mut KDNode, point: Vec<f32>, id: usize, meta: String, depth: usize) {
        let axis = depth % self.dimension;
        if point[axis] < node.point[axis] {
            if node.left.is_some() {
                self.insert_rec(node.left.as_mut().unwrap(), point, id, meta, depth + 1);
            } else {
                node.left = Some(Box::new(KDNode {
                    point,
                    id,
                    metadata: meta,
                    left: None,
                    right: None,
                }));
            }
        } else {
            if node.right.is_some() {
                self.insert_rec(node.right.as_mut().unwrap(), point, id, meta, depth + 1);
            } else {
                node.right = Some(Box::new(KDNode {
                    point,
                    id,
                    metadata: meta,
                    left: None,
                    right: None,
                }));
            }
        }
    }

    pub fn query(&self, query_vec: &Vec<f32>, k: usize) -> Vec<(usize, f32)> {
        if self.root.is_none() || query_vec.len() != self.dimension {
            return Vec::new();
        }
        let mut pq: BinaryHeap<Reverse<(f32, usize)>> = BinaryHeap::new();
        self.knn_search(self.root.as_ref().unwrap(), query_vec, 0, k, &mut pq);
        let mut results: Vec<(usize, f32)> = pq.into_iter().map(|Reverse((dist, id))| (id, dist)).collect();
        results.sort_by(|a, b| a.1.partial_cmp(&b.1).unwrap());
        results
    }

    fn knn_search(&self, node: &KDNode, query: &Vec<f32>, depth: usize, k: usize, pq: &mut BinaryHeap<Reverse<(f32, usize)>>) {
        let dist = Self::distance(&node.point, query);
        if pq.len() < k {
            pq.push(Reverse((dist, node.id)));
        } else if dist < pq.peek().unwrap().0.0 {
            pq.pop();
            pq.push(Reverse((dist, node.id)));
        }

        let axis = depth % self.dimension;
        let (next_branch, other_branch) = if query[axis] < node.point[axis] {
            (&node.left, &node.right)
        } else {
            (&node.right, &node.left)
        };

        if let Some(nb) = next_branch {
            self.knn_search(nb, query, depth + 1, k, pq);
        }

        if let Some(ob) = other_branch {
            let axis_diff = (query[axis] - node.point[axis]).abs();
            if pq.len() < k || axis_diff <= pq.peek().unwrap().0.0 {
                self.knn_search(ob, query, depth + 1, k, pq);
            }
        }
    }

    pub fn get_metadata(&self, id: usize) -> Option<String> {
        self.find_node(self.root.as_ref()?, id, 0).map(|node| node.metadata.clone())
    }

    fn find_node(&self, node: &KDNode, id: usize, depth: usize) -> Option<&KDNode> {
        if node.id == id {
            return Some(node);
        }
        let axis = depth % self.dimension;
        let branch = if self.all_vectors[id][axis] < node.point[axis] {
            &node.left
        } else {
            &node.right
        };
        branch.as_ref()?.as_ref().and_then(|n| self.find_node(n, id, depth + 1))
    }

    pub fn save_to_file(&self, filename: &str) -> Result<(), Box<dyn std::error::Error>> {
        let data = (self.dimension, self.next_id, &self.all_vectors, &self.root);
        let encoded = bincode::serialize(&data)?;
        std::fs::write(filename, encoded)?;
        Ok(())
    }

    pub fn load_from_file(&mut self, filename: &str) -> Result<(), Box<dyn std::error::Error>> {
        let data: (usize, usize, Vec<Vec<f32>>, Option<Box<KDNode>>) = bincode::deserialize(&std::fs::read(filename)?)?;
        self.dimension = data.0;
        self.next_id = data.1;
        self.all_vectors = data.2;
        self.root = data.3;
        Ok(())
    }

    pub fn size(&self) -> usize {
        self.next_id
    }
}
